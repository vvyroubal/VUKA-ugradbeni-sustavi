#!/usr/bin/env bash
# run_qemu_gdb.sh — boot an ESP-IDF esp32c3 app build in QEMU, halted, with the
# GDB stub open for a debugger to attach.
#
# Usage: run_qemu_gdb.sh <build_dir>
#
# <build_dir> is an IDF project's `build/` directory (contains flash_args and
# the app/bootloader/partition-table binaries produced by `idf.py build`) —
# same merge logic as run_qemu.sh: (re)merge <build_dir>/flash.bin from
# <build_dir>/flash_args via esptool.py (chip esp32c3, 4MB flash image) on
# every run, so a rebuild is always what boots.
#
# Unlike run_qemu.sh, this is an INTERACTIVE tool with no timeout: QEMU is
# started with `-s -S` (gdbstub on tcp::1234, CPU halted at reset) in the
# foreground, and this script prints the matching connect command before
# exec'ing QEMU so it's the last thing visible above the QEMU/gdb session:
#
#   riscv32-esp-elf-gdb <build_dir>/<project>.elf -ex 'target remote :1234'
#
# Run this in one terminal, then run the printed gdb command in a second
# terminal on the same machine. With Docker, open the second terminal with
# ./scripts/docker-shell.sh as well: while a course container is running it
# attaches to that same container, so :1234 is reachable.
#
# Requires esptool.py and qemu-system-riscv32 on PATH (both are on PATH inside
# the embedded-course Docker image after the IDF export environment is
# active, which is the default entrypoint behaviour of espressif/idf images).

set -u

usage() {
  echo "Usage: $0 <build_dir>" >&2
}

if [ "$#" -ne 1 ]; then
  usage
  exit 1
fi

BUILD_DIR="$1"

if [ ! -d "$BUILD_DIR" ]; then
  echo "run_qemu_gdb.sh: build dir not found: $BUILD_DIR" >&2
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
    echo "run_qemu_gdb.sh: no flash.bin and no flash_args in $BUILD_DIR" >&2
    exit 1
  fi
  if ! command -v esptool.py >/dev/null 2>&1; then
    echo "run_qemu_gdb.sh: esptool.py not found on PATH" >&2
    exit 1
  fi
  echo "run_qemu_gdb.sh: merging $FLASH_BIN from $FLASH_ARGS ..." >&2
  (
    cd "$BUILD_DIR" &&
    esptool.py --chip esp32c3 merge_bin -o "$(basename "$FLASH_BIN")" \
      --fill-flash-size 4MB @flash_args --format raw
  )
  if [ ! -f "$FLASH_BIN" ]; then
    echo "run_qemu_gdb.sh: merge_bin failed to produce $FLASH_BIN" >&2
    exit 1
  fi
fi

if ! command -v qemu-system-riscv32 >/dev/null 2>&1; then
  echo "run_qemu_gdb.sh: qemu-system-riscv32 not found on PATH" >&2
  exit 1
fi

# Find the project .elf next to flash.bin (there is exactly one at the top of
# an IDF build dir: <project_name>.elf).
ELF_FILE="$(find "$BUILD_DIR" -maxdepth 1 -name '*.elf' | head -n1)"
if [ -z "$ELF_FILE" ]; then
  echo "run_qemu_gdb.sh: no .elf found directly in $BUILD_DIR" >&2
  exit 1
fi

: > "$LOG_FILE"

echo "==============================================================" >&2
echo "run_qemu_gdb.sh: QEMU starting HALTED with gdbstub on :1234" >&2
echo "In a second terminal, connect with the command below (Docker: run" >&2
echo "./scripts/docker-shell.sh there too -- it attaches to this container):" >&2
echo "==============================================================" >&2
echo "riscv32-esp-elf-gdb $ELF_FILE -ex 'target remote :1234'"

exec qemu-system-riscv32 -M esp32c3 -nographic -s -S \
    -drive file="$FLASH_BIN",if=mtd,format=raw 2>&1 | tee "$LOG_FILE"
