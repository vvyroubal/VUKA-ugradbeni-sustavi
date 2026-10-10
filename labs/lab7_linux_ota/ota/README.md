# Lab 7 — signed A/B OTA (host tooling)

This directory holds the **host-side** tooling for the Lab 7 over-the-air update
exercise. The **target-side** program (`ota-apply`) and the trust anchor
(`/etc/lab7/ota_pub.pem`) live in the rootfs overlay
(`../buildroot/board/lab7/rootfs_overlay/`), because they run on the emulated
device, not on your workstation.

The whole scheme is **deliberately simple and fully inspectable** — every step is
a few lines of shell you can read. It mirrors the *shape* of a real A/B OTA
(sign the image, verify on-device, update the inactive slot, switch on reboot,
roll back on failure) while honestly simplifying the parts a real bootloader
owns. Those simplifications are called out below and in the manual.

## Files

| File | Runs on | Role |
|------|---------|------|
| `make_bundle.sh` | host | build a signed update bundle from a rootfs image |
| `ota-boot-select.sh` | host | manage the A/B boot env (init slots, get/set/toggle) |
| `keys/` | host | the **demo** signing keypair (see the warning below) |
| `../buildroot/.../usr/bin/ota-apply` | target | verify a bundle and install it to the inactive slot |
| `../buildroot/.../usr/bin/lab7-selftest` | target | print `LAB7 OK version=N slot=x` |

## The disk / slot layout

`run_virt.sh` presents up to four virtio disks to the guest:

| Device | Backing file | Meaning |
|--------|--------------|---------|
| `/dev/vda` | active slot (`rootfs_a` or `rootfs_b`) | the running root filesystem (read-only) |
| `/dev/vdb` | inactive slot | `ota-apply`'s write target |
| `/dev/vdc` | `bootenv` (1 byte: `a`/`b`) | boot-select env — the "bootloader environment" stand-in |
| `/dev/vdd` | an OTA bundle (optional) | passed with `LAB7_VDD=<bundle>` |

Which physical file is `vda` vs `vdb` is decided by the **host** runner from the
active letter in `bootenv`. `ota-apply` never needs to know the filenames — it
always writes `/dev/vdb` and flips `/dev/vdc`.

## Bundle format

A plain tar (so you can `tar tvf` it) with three members:

```
rootfs.ext4        the new root filesystem image
rootfs.ext4.sig    RSA-SHA256 signature of rootfs.ext4
version            human-readable version string
```

## The update cycle

```sh
# 0. build the Buildroot image once (see ../Makefile.buildroot), then:
./ota-boot-select.sh init                 # seed rootfs_a/b, bootenv=a

# 1. build a signed v2 bundle (slot b will become active)
./make_bundle.sh --in ../artifacts/rootfs.ext4 --version 2.0.0 --slot b \
                 --out ../artifacts/ota_v2.tar

# 2. boot slot A, apply the bundle, power off
LAB7_VDD=../artifacts/ota_v2.tar ../run_virt.sh          # login root/root
#   guest:  ota-apply /dev/vdd     -> verifies, writes /dev/vdb, flips bootenv

# 3. reboot — the runner now selects slot B
../run_virt.sh
#   guest:  cat /etc/lab7-release  -> LAB7_VERSION=2.0.0 / LAB7_SLOT=B
#           lab7-selftest          -> LAB7 OK version=2 slot=b
```

`../qemu_smoke.sh` automates exactly this (plus the tamper test) non-interactively.

## Tamper test

Flip one byte inside a bundle's `rootfs.ext4` region and re-apply: the signature
no longer matches the image, so `ota-apply` prints `LAB7 OK tamper=rejected`,
exits non-zero, and leaves the inactive slot and boot env untouched.

## What this honestly simplifies away

- **Boot selection is a host file**, not a bootloader reading slot metadata. A
  real device (e.g. the ESP32-C3's second-stage bootloader, or U-Boot) reads an
  environment/partition to pick a slot and can auto-roll-back on boot failure.
  Here the host runner reads `bootenv`; there is no automatic rollback.
- **The trust anchor is a public key in the rootfs**, which is honest, but a real
  product anchors it in immutable storage (the C3 burns a public-key *hash* into
  eFuses so the key itself can't be swapped). Here nothing stops someone editing
  `/etc/lab7/ota_pub.pem` in the image — the point is to show *where* the anchor
  sits, not to be unbreakable.
- **No anti-rollback / version pinning, no encryption** — we verify integrity and
  authenticity of the image, nothing more.

## ⚠️ The signing key here is a throwaway demo key

`keys/demo_signing_key.INSECURE.pem` is committed **on purpose** so the lab and
its smoke test are reproducible out of the box. That means it is *public* and
provides **no security** — anyone can forge a "valid" bundle with it.

A real product does the opposite: the **private** key lives offline (an HSM or an
air-gapped signer), is **never** committed or shipped, and only the public key
travels onto the device. To practice that, generate your own keypair, drop the
public half into the overlay, rebuild the rootfs, and sign with `--key`:

```sh
openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -out my_key.pem
openssl rsa -in my_key.pem -pubout \
  -out ../buildroot/board/lab7/rootfs_overlay/etc/lab7/ota_pub.pem
make -C .. -f Makefile.buildroot rootfs      # re-bake the new public key
./make_bundle.sh --key my_key.pem  ...
```
