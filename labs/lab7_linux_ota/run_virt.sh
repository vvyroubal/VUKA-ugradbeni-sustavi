#!/usr/bin/env bash
# run_virt.sh — boot the Lab 7 Buildroot RISC-V image on QEMU's generic 'virt' board.
#
# Usage: run_virt.sh [artifacts_dir] [timeout_seconds]
#   artifacts_dir   default: <script_dir>/artifacts  (produced by Makefile.buildroot)
#   timeout_seconds default: 0 = interactive (no timeout). A positive value bounds
#                   the run with `timeout` — used by non-interactive smoke checks.
#
# QEMU binary: this lab needs UPSTREAM qemu-system-riscv32 (Debian's
# qemu-system-misc, at /usr/bin). The embedded-course container also ships
# Espressif's QEMU fork as qemu-system-riscv32 (for the ESP32-C3 labs) and that
# fork shadows the name on PATH — but it CANNOT run the generic 'virt' board:
# although `-M help` lists "virt", the fork specialises the RISC-V CPU
# (espressif-riscv-cpu) and aborts on virt's generic rv32 cpu
# ("esp_cpu_riscv_to_cpu: ... not an instance of type espressif-riscv-cpu").
# So we deliberately pick a non-Espressif binary below (override with
# QEMU_RISCV32=/path). On Linux dev hosts / CI, qemu-system-riscv32 on PATH is
# already upstream and is used as-is.
#
# Boot recipe: this riscv32 'virt' target boots via OpenSBI (fw_jump.bin) which
# then jumps to the Linux -kernel. The Buildroot board readme's proven invocation
# is followed (with an explicit virtio-blk-device so the rootfs is deterministically
# /dev/vda). The task brief's sketch invocation was adapted minimally:
#   + added  -bios fw_jump.bin        (OpenSBI firmware — the defconfig builds it;
#                                       without it the SBI/kernel does not come up)
#   + used   rootwait root=/dev/vda   (rootwait: wait for the virtio-blk probe)
#   + used   -drive ...,if=none + -device virtio-blk-device,drive=hd0
#                                     (explicit virtio-blk → stable /dev/vda; the
#                                       bare `-drive ...,if=virtio` sketch is not a
#                                       valid interface on the riscv 'virt' board)
#   + used   -netdev user,id=net0 -device virtio-net-device,netdev=net0
#                                     ('virt' has no built-in NIC; the user-net
#                                       device must be attached explicitly)
#
# Login: root / root  (BR2_TARGET_GENERIC_ROOT_PASSWD in lab7_defconfig).
# Verify marker:  cat /etc/lab7-release   ->  LAB7_VERSION=1.0.0 / LAB7_SLOT=A

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ART_DIR="${1:-$SCRIPT_DIR/artifacts}"
TIMEOUT="${2:-0}"

KERNEL="$ART_DIR/Image"
BIOS="$ART_DIR/fw_jump.bin"

for f in "$KERNEL" "$BIOS"; do
  if [ ! -f "$f" ]; then
    echo "run_virt.sh: missing artifact '$f'." >&2
    echo "Build it first:  make -f Makefile.buildroot all   (inside embedded-course)" >&2
    exit 1
  fi
done

# Disk selection — two modes:
#   A/B    : if bootenv + both slot images exist, boot the ACTIVE slot as
#            /dev/vda, attach the INACTIVE slot as /dev/vdb (ota-apply's write
#            target) and the boot env as /dev/vdc. This is the OTA-cycle layout.
#   single : otherwise boot the plain rootfs.ext4 as /dev/vda (Task-12 boot test).
# An optional OTA bundle is attached as /dev/vdd when LAB7_VDD=<file> is set.
BOOTENV="$ART_DIR/bootenv"
SLOT_A="$ART_DIR/rootfs_a.ext4"
SLOT_B="$ART_DIR/rootfs_b.ext4"

if [ -f "$BOOTENV" ] && [ -f "$SLOT_A" ] && [ -f "$SLOT_B" ]; then
  active=$(head -c1 "$BOOTENV")
  case "$active" in
    a) VDA="$SLOT_A"; VDB="$SLOT_B" ;;
    b) VDA="$SLOT_B"; VDB="$SLOT_A" ;;
    *) echo "run_virt.sh: bad bootenv '$active' (want 'a'/'b')" >&2; exit 1 ;;
  esac
  MODE="A/B (active slot $active)"
else
  ROOTFS="$ART_DIR/rootfs.ext4"
  if [ ! -f "$ROOTFS" ]; then
    echo "run_virt.sh: missing artifact '$ROOTFS'." >&2
    echo "Build it first:  make -f Makefile.buildroot all   (inside embedded-course)" >&2
    exit 1
  fi
  VDA="$ROOTFS"; VDB=""
  MODE="single-slot"
fi

# Pick a qemu-system-riscv32 that is NOT the Espressif fork (see header). Honour
# an explicit QEMU_RISCV32 override; else try the upstream Debian binary and any
# on PATH, skipping builds whose --version advertises the esp fork.
QEMU_BIN=""
for cand in "${QEMU_RISCV32:-}" /usr/bin/qemu-system-riscv32 qemu-system-riscv32; do
  [ -n "$cand" ] || continue
  command -v "$cand" >/dev/null 2>&1 || continue
  if "$cand" --version 2>/dev/null | grep -qi esp_develop; then
    continue   # Espressif fork — cannot boot the generic 'virt' board
  fi
  QEMU_BIN="$cand"
  break
done

if [ -z "$QEMU_BIN" ]; then
  echo "run_virt.sh: no upstream qemu-system-riscv32 found (only the Espressif fork," >&2
  echo "  which cannot run '-M virt'). Install Debian's qemu-system-misc, or set" >&2
  echo "  QEMU_RISCV32=/path/to/upstream/qemu-system-riscv32." >&2
  exit 1
fi

# Build the disk list in /dev/vd{a,b,c,d} order (virtio-mmio slots are assigned in
# -device order): vda = active root, then vdb = inactive slot, vdc = boot env,
# vdd = optional bundle.
QEMU_ARGS=(
  -M virt
  -m 256M
  -bios "$BIOS"
  -kernel "$KERNEL"
  -append "rootwait root=/dev/vda ro"
  -drive "file=$VDA,format=raw,id=hd0,if=none"
  -device virtio-blk-device,drive=hd0
)
if [ -n "$VDB" ]; then
  QEMU_ARGS+=( -drive "file=$VDB,format=raw,id=hd1,if=none"     -device virtio-blk-device,drive=hd1 )
  QEMU_ARGS+=( -drive "file=$BOOTENV,format=raw,id=env,if=none" -device virtio-blk-device,drive=env )
fi
if [ -n "${LAB7_VDD:-}" ]; then
  [ -f "$LAB7_VDD" ] || { echo "run_virt.sh: LAB7_VDD='$LAB7_VDD' not found" >&2; exit 1; }
  QEMU_ARGS+=( -drive "file=$LAB7_VDD,format=raw,id=bundle,if=none" -device virtio-blk-device,drive=bundle )
fi
QEMU_ARGS+=(
  -netdev user,id=net0
  -device virtio-net-device,netdev=net0
  -nographic
)

echo "== booting Lab 7 riscv32 virt image [$MODE] via $QEMU_BIN (login: root/root; Ctrl-A X to quit QEMU)"
if [ "$TIMEOUT" -gt 0 ] 2>/dev/null; then
  # --foreground: see labs/common/run_qemu.sh (QEMU stops on SIGTTOU otherwise)
  exec timeout --foreground "$TIMEOUT" "$QEMU_BIN" "${QEMU_ARGS[@]}"
else
  exec "$QEMU_BIN" "${QEMU_ARGS[@]}"
fi
