#!/usr/bin/env bash
# run_qemu_net.sh — like run_qemu.sh, but attaches QEMU user-mode networking
# with the emulated OpenCores Ethernet NIC (open_eth) so the booted app can do
# IP networking (DHCP + TCP) inside QEMU.
#
# Usage: run_qemu_net.sh <build_dir> [timeout=40]
#
# This is run_qemu.sh's boot-and-tee logic plus a single extra QEMU flag:
#   -nic user,model=open_eth
# which gives the guest a SLIRP user-mode network: the guest DHCPs 10.0.2.15,
# the gateway/host alias is 10.0.2.2, and TCP connections to 10.0.2.2:<port>
# are NAT-forwarded to services listening on the QEMU *host's* loopback (e.g.
# a broker started by qemu_smoke.sh in the same container). Proven on esp32c3
# in this QEMU build — see docs/decisions/qemu-network.md ("data-plane spike").
#
# run_qemu.sh deliberately stays network-free per its own contract; this is the
# networked sibling used by Lab 6. Exit-code semantics are identical to
# run_qemu.sh: 0 on clean QEMU exit, or on our own timeout AFTER serial output
# was produced (the normal case for an app that loops forever); 1 otherwise.
#
# Requires esptool.py and qemu-system-riscv32 on PATH (both present inside the
# embedded-course Docker image after the IDF export environment is active).

set -u

usage() {
  echo "Usage: $0 <build_dir> [timeout=40]" >&2
}

if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
  usage
  exit 1
fi

BUILD_DIR="$1"
TIMEOUT="${2:-40}"

if [ ! -d "$BUILD_DIR" ]; then
  echo "run_qemu_net.sh: build dir not found: $BUILD_DIR" >&2
  exit 1
fi

FLASH_ARGS="$BUILD_DIR/flash_args"
FLASH_BIN="$BUILD_DIR/flash.bin"
LOG_FILE="$BUILD_DIR/qemu.log"

# Always re-merge when the build's flash_args is there: `idf.py build` does
# not remove an older flash.bin, and silently booting the previous image
# after a rebuild is the most confusing failure a student can hit.
if [ -f "$FLASH_ARGS" ]; then
  rm -f "$FLASH_BIN"
fi

if [ ! -f "$FLASH_BIN" ]; then
  if [ ! -f "$FLASH_ARGS" ]; then
    echo "run_qemu_net.sh: no flash.bin and no flash_args in $BUILD_DIR" >&2
    exit 1
  fi
  if ! command -v esptool.py >/dev/null 2>&1; then
    echo "run_qemu_net.sh: esptool.py not found on PATH" >&2
    exit 1
  fi
  echo "run_qemu_net.sh: merging $FLASH_BIN from $FLASH_ARGS ..." >&2
  (
    cd "$BUILD_DIR" &&
    esptool.py --chip esp32c3 merge_bin -o "$(basename "$FLASH_BIN")" \
      --fill-flash-size 4MB @flash_args --format raw
  )
  if [ ! -f "$FLASH_BIN" ]; then
    echo "run_qemu_net.sh: merge_bin failed to produce $FLASH_BIN" >&2
    exit 1
  fi
fi

if ! command -v qemu-system-riscv32 >/dev/null 2>&1; then
  echo "run_qemu_net.sh: qemu-system-riscv32 not found on PATH" >&2
  exit 1
fi

# macOS ships no `timeout`; Homebrew coreutils installs it as `gtimeout`.
TIMEOUT_BIN="$(command -v timeout || command -v gtimeout || true)"
if [ -z "$TIMEOUT_BIN" ]; then
  echo "run_qemu_net.sh: neither 'timeout' nor 'gtimeout' found on PATH" \
    "(on macOS: brew install coreutils)" >&2
  exit 1
fi

: > "$LOG_FILE"

"$TIMEOUT_BIN" "$TIMEOUT" qemu-system-riscv32 -M esp32c3 -nographic \
    -nic user,model=open_eth \
    -drive file="$FLASH_BIN",if=mtd,format=raw 2>&1 | tee "$LOG_FILE"
QEMU_STATUS="${PIPESTATUS[0]}"

if [ "$QEMU_STATUS" -eq 0 ]; then
  exit 0
elif [ "$QEMU_STATUS" -eq 124 ]; then
  if [ -s "$LOG_FILE" ]; then
    echo "run_qemu_net.sh: timeout (${TIMEOUT}s) reached after output was produced — treating as success" >&2
    exit 0
  else
    echo "run_qemu_net.sh: timeout (${TIMEOUT}s) reached with no output — treating as failure" >&2
    exit 1
  fi
else
  echo "run_qemu_net.sh: qemu-system-riscv32 exited with status $QEMU_STATUS" >&2
  exit 1
fi
