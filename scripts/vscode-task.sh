#!/usr/bin/env bash
# vscode-task.sh -- runs one course action for a lab project; the VS Code
# tasks in .vscode/tasks.json call it (Terminal > Run Task...), and it works
# from any terminal too.
#
# Usage (from anywhere):
#     bash scripts/vscode-task.sh <action> <project>
#   action:  build | qemu | build-qemu | menuconfig | reconfigure | fullclean
#   project: lab project relative to the repository root,
#            e.g. labs/lab1_toolchain/starter
#
# Where ESP-IDF comes from, in this order:
#   1. it is already active (an activated terminal, or VS Code attached to
#      the course container);
#   2. $IDF_PATH/export.sh, then ~/esp-idf/export.sh (path B, handout H1);
#   3. otherwise the course container, through scripts/docker-shell.sh
#      (path A) -- it attaches if that container is already running.

set -eo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

usage() {
  echo "Usage: bash scripts/vscode-task.sh <build|qemu|build-qemu|menuconfig|reconfigure|fullclean> <project>" >&2
  exit 1
}
[ "$#" -eq 2 ] || usage
ACTION="$1"
PROJ="${2%/}"
if [ ! -f "$ROOT/$PROJ/CMakeLists.txt" ]; then
  echo "vscode-task.sh: not an ESP-IDF project: $PROJ" >&2
  exit 1
fi

# Lab 6 needs the emulated network card, the other labs do not.
QEMU_SCRIPT=run_qemu.sh
case "$PROJ" in labs/lab6_mqtt/*) QEMU_SCRIPT=run_qemu_net.sh ;; esac

case "$ACTION" in
  build)       CMD="idf.py build" ;;
  qemu)        CMD="bash ../../common/$QEMU_SCRIPT build" ;;
  build-qemu)  CMD="idf.py build && bash ../../common/$QEMU_SCRIPT build" ;;
  menuconfig)  CMD="idf.py menuconfig" ;;
  reconfigure) CMD="idf.py reconfigure" ;;
  fullclean)   CMD="idf.py fullclean" ;;
  *)           usage ;;
esac

if ! command -v idf.py >/dev/null 2>&1; then
  for export_sh in "${IDF_PATH:-/nonexistent}/export.sh" "$HOME/esp-idf/export.sh"; do
    if [ -f "$export_sh" ]; then
      # shellcheck disable=SC1090
      . "$export_sh" >/dev/null 2>&1 || true
      break
    fi
  done
fi

if command -v idf.py >/dev/null 2>&1; then
  echo "== $CMD   (in $PROJ)"
  cd "$ROOT/$PROJ"
  exec bash -c "$CMD"
fi

if command -v docker >/dev/null 2>&1; then
  echo "== $CMD   (in $PROJ, course container)"
  cd "$ROOT"
  exec ./scripts/docker-shell.sh bash -c "cd $PROJ && $CMD"
fi

echo "vscode-task.sh: no ESP-IDF found (H1: path B, ~/esp-idf) and no docker (H1: path A)" >&2
exit 1
