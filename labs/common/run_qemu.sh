#!/usr/bin/env bash
# run_qemu.sh — boot an ESP-IDF esp32c3 app build in QEMU and capture serial output.
#
# Usage: run_qemu.sh <build_dir> [timeout=30]
#
# <build_dir> is an IDF project's `build/` directory (contains flash_args and the
# app/bootloader/partition-table binaries produced by `idf.py build`).
#
# Behaviour:
#   - <build_dir>/flash.bin is (re)merged from <build_dir>/flash_args via
#     esptool.py (chip esp32c3, 4MB flash image) on every run, so the image
#     that boots is always the latest build.
#   - Boots `qemu-system-riscv32 -M esp32c3 -nographic` with that flash image
#     attached as raw MTD, tee-ing serial output to <build_dir>/qemu.log.
#   - QEMU is bounded by `timeout <timeout>`.
#
# Exit status:
#   0  if QEMU exited on its own (clean exit, status 0) OR if the timeout was
#      reached AFTER qemu.log has some output (i.e. the target booted and was
#      still running — this is the expected/normal case for apps that loop
#      forever, e.g. hello_world's restart loop).
#   1  otherwise (missing build dir, merge failure, or QEMU exited non-zero
#      for a reason other than our own timeout, or timeout reached with an
#      empty log — meaning QEMU never produced any output at all).
#
# Requires esptool.py and qemu-system-riscv32 on PATH (both are on PATH inside
# the embedded-course Docker image after the IDF export environment is active,
# which is the default entrypoint behaviour of espressif/idf images).

set -u

usage() {
  echo "Usage: $0 <build_dir> [timeout=30]" >&2
}

if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
  usage
  exit 1
fi

BUILD_DIR="$1"
TIMEOUT="${2:-30}"

if [ ! -d "$BUILD_DIR" ]; then
  echo "run_qemu.sh: build dir not found: $BUILD_DIR" >&2
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
    echo "run_qemu.sh: no flash.bin and no flash_args in $BUILD_DIR" >&2
    exit 1
  fi
  if ! command -v esptool.py >/dev/null 2>&1; then
    echo "run_qemu.sh: esptool.py not found on PATH" >&2
    exit 1
  fi
  echo "run_qemu.sh: merging $FLASH_BIN from $FLASH_ARGS ..." >&2
  (
    cd "$BUILD_DIR" &&
    esptool.py --chip esp32c3 merge_bin -o "$(basename "$FLASH_BIN")" \
      --fill-flash-size 4MB @flash_args --format raw
  )
  if [ ! -f "$FLASH_BIN" ]; then
    echo "run_qemu.sh: merge_bin failed to produce $FLASH_BIN" >&2
    exit 1
  fi
fi

if ! command -v qemu-system-riscv32 >/dev/null 2>&1; then
  echo "run_qemu.sh: qemu-system-riscv32 not found on PATH" >&2
  exit 1
fi

# GNU coreutils' `timeout` is not guaranteed to exist on macOS: the BSD
# userland ships no `timeout` at all, and Homebrew's coreutils installs it
# under the `gtimeout` name to avoid clashing with the system tools. Resolve
# whichever one is actually on PATH instead of hard-coding `timeout`.
TIMEOUT_BIN="$(command -v timeout || command -v gtimeout || true)"
if [ -z "$TIMEOUT_BIN" ]; then
  echo "run_qemu.sh: neither 'timeout' nor 'gtimeout' found on PATH" \
    "(on macOS: brew install coreutils)" >&2
  exit 1
fi

: > "$LOG_FILE"

"$TIMEOUT_BIN" "$TIMEOUT" qemu-system-riscv32 -M esp32c3 -nographic \
    -drive file="$FLASH_BIN",if=mtd,format=raw 2>&1 | tee "$LOG_FILE"
QEMU_STATUS="${PIPESTATUS[0]}"

# `timeout` exits 124 when it had to kill the process after the time budget.
# That is our normal/expected outcome for apps that loop forever (e.g.
# hello_world's restart loop) — treat it as success as long as QEMU actually
# produced serial output before being killed. Any other non-zero status is a
# real failure (QEMU crash, missing device, bad image, etc).
if [ "$QEMU_STATUS" -eq 0 ]; then
  exit 0
elif [ "$QEMU_STATUS" -eq 124 ]; then
  if [ -s "$LOG_FILE" ]; then
    echo "run_qemu.sh: timeout (${TIMEOUT}s) reached after output was produced — treating as success" >&2
    exit 0
  else
    echo "run_qemu.sh: timeout (${TIMEOUT}s) reached with no output — treating as failure" >&2
    exit 1
  fi
else
  echo "run_qemu.sh: qemu-system-riscv32 exited with status $QEMU_STATUS" >&2
  exit 1
fi
