# ⚠️ Demo signing key — NOT a secret

`demo_signing_key.INSECURE.pem` is a throwaway RSA private key committed **on
purpose** so the Lab 7 OTA exercise and its `qemu_smoke.sh` run reproducibly out
of the box. Because it is public it provides **zero security** — anyone can forge
a bundle that verifies.

- `demo_signing_key.INSECURE.pem` — private key, used by `../make_bundle.sh`
- `ota_signing_pub.pem` — matching public key (also baked into the rootfs at
  `/etc/lab7/ota_pub.pem`, where the target uses it as the trust anchor)

**Never** commit or ship a real product's private signing key. It belongs offline
(HSM / air-gapped signer); only the public half goes on the device. See the
"generate your own keypair" recipe in `../README.md` to practise the real flow.
