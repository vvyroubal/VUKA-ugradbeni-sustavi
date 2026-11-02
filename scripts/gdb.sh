#!/usr/bin/env bash
# gdb.sh -- starts riscv32-esp-elf-gdb from the ESP-IDF installation with the
# given arguments. The VS Code debug configuration (.vscode/launch.json) uses
# it as its debugger: VS Code itself has no activated ESP-IDF, so the
# debugger is not on its PATH. Prints nothing on stdout, which the debugger
# extension reads as GDB/MI.
#
# Where GDB comes from, in this order:
#   1. it is already on PATH (an activated terminal, the course container);
#   2. $IDF_TOOLS_PATH (default ~/.espressif; /opt/esp in the container),
#      as installed by ESP-IDF's install.sh (path B, handout H1).
#
# Path A: VS Code must be attached to the course container (README, "Rad u
# VS Codeu"), so that this script, GDB and QEMU all run in the container.

set -u
GDB=riscv32-esp-elf-gdb

if ! command -v "$GDB" >/dev/null 2>&1; then
  found=""
  for g in "${IDF_TOOLS_PATH:-$HOME/.espressif}"/tools/riscv32-esp-elf-gdb/*/riscv32-esp-elf-gdb/bin/riscv32-esp-elf-gdb; do
    [ -x "$g" ] && found="$g"
  done
  if [ -z "$found" ]; then
    echo "gdb.sh: riscv32-esp-elf-gdb not found (H1: path B, or attach VS Code to the course container for path A)" >&2
    exit 1
  fi
  GDB="$found"
fi

exec "$GDB" "$@"
