#!/usr/bin/env bash
# ota-boot-select.sh — manage the Lab 7 A/B boot state on the host.
#
# This is the host-side stand-in for a bootloader's slot-selection environment.
# The "boot env" is a 512-byte file `bootenv` in the artifacts dir whose first
# byte is the active slot letter ('a' or 'b'); run_virt.sh reads it to decide
# which slot image becomes /dev/vda, and the target's ota-apply flips it (over
# the /dev/vdc virtio device backed by this same file) to mark a new slot active.
#
# Usage:
#   ota-boot-select.sh init            [--dir ARTIFACTS]   # make slots + bootenv=a
#   ota-boot-select.sh get             [--dir ARTIFACTS]   # print active slot
#   ota-boot-select.sh set <a|b>       [--dir ARTIFACTS]
#   ota-boot-select.sh toggle          [--dir ARTIFACTS]
#
# `init` seeds both slots from artifacts/rootfs.ext4 (the freshly built image)
# and sets slot A active — the starting point for the OTA cycle.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ART_DIR="$(cd "$SCRIPT_DIR/.." && pwd)/artifacts"

CMD="${1:-}"; [ $# -gt 0 ] && shift || true
# optional --dir override (may appear after the sub-args)
ARGS=()
while [ $# -gt 0 ]; do
  case "$1" in
    --dir) ART_DIR="$2"; shift 2 ;;
    *) ARGS+=("$1"); shift ;;
  esac
done

BOOTENV="$ART_DIR/bootenv"
ROOTFS="$ART_DIR/rootfs.ext4"
SLOT_A="$ART_DIR/rootfs_a.ext4"
SLOT_B="$ART_DIR/rootfs_b.ext4"

write_bootenv() { # $1 = letter (a|b)  -> 512-byte file, byte 0 = letter
  { printf '%s' "$1"; dd if=/dev/zero bs=1 count=511 2>/dev/null; } > "$BOOTENV"
}
read_bootenv() { head -c1 "$BOOTENV"; }

case "$CMD" in
  init)
    [ -f "$ROOTFS" ] || { echo "ota-boot-select: $ROOTFS not found (build the image first)" >&2; exit 1; }
    cp -f "$ROOTFS" "$SLOT_A"
    cp -f "$ROOTFS" "$SLOT_B"
    write_bootenv a
    echo "ota-boot-select: initialised slots A/B from rootfs.ext4; active = a"
    ;;
  get)
    [ -f "$BOOTENV" ] || { echo "ota-boot-select: no bootenv (run 'init')" >&2; exit 1; }
    read_bootenv; echo
    ;;
  set)
    case "${ARGS[0]:-}" in a|b) write_bootenv "${ARGS[0]}"; echo "ota-boot-select: active = ${ARGS[0]}" ;;
      *) echo "ota-boot-select: 'set' needs a|b" >&2; exit 2 ;; esac
    ;;
  toggle)
    cur=$(read_bootenv)
    case "$cur" in a) write_bootenv b; echo "ota-boot-select: active = b" ;;
                   b) write_bootenv a; echo "ota-boot-select: active = a" ;;
                   *) echo "ota-boot-select: bad bootenv '$cur'" >&2; exit 2 ;; esac
    ;;
  *) sed -n '9,15p' "$0"; exit 2 ;;
esac
