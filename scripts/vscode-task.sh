#!/usr/bin/env bash
# vscode-task.sh -- runs one course action for a lab project; the VS Code
# tasks in .vscode/tasks.json call it (Terminal > Run Task...), and it works
# from any terminal too.
#
# Usage (from anywhere):
#     bash scripts/vscode-task.sh <action> <project>
#   action:  build | qemu | build-qemu | build-vcd | gdb | build-gdb
#            | menuconfig | reconfigure | fullclean
#            (build-vcd: build, then QEMU with --gpio-vcd -- one
#            build/gpio/gpioN.vcd per output pin, LV1 and LV2;
#            gdb: the latest build in QEMU, halted, waiting for a debugger
#            on :1234; build-gdb: build first -- .vscode/launch.json)
#   or:      bash scripts/vscode-task.sh lab7
#            (Lab 7: Buildroot build, 30-60 min the first time)
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
  echo "Usage: bash scripts/vscode-task.sh <build|qemu|build-qemu|build-vcd|gdb|build-gdb|menuconfig|reconfigure|fullclean> <project>" >&2
  exit 1
}
# Lab 7 is a Buildroot (Linux) build, not an ESP-IDF project. Where it runs:
#   - VS Code attached to the course container: in the container, in
#     /br (the lab7_br volume) when /work is a macOS/Windows bind mount,
#     which is not case-sensitive; otherwise in labs/lab7_linux_ota/.br;
#   - a Linux host with the course image: through scripts/docker-shell.sh,
#     as the README shows; without it, natively (path B, extra packages);
#   - a macOS host: in the course container, in /br.
if [ "${1:-}" = lab7 ]; then
  MK="make -C labs/lab7_linux_ota -f Makefile.buildroot all"
  cd "$ROOT"
  if [ -f /.dockerenv ]; then
    probe="$(mktemp "$ROOT/.case-probe-XXXXXX")"
    upper="$(dirname "$probe")/$(basename "$probe" | tr '[:lower:]' '[:upper:]')"
    insensitive=""; [ -e "$upper" ] && insensitive=1
    rm -f "$probe"
    if [ -n "$insensitive" ]; then
      if [ ! -d /br ]; then
        echo "vscode-task.sh: /br is missing -- exit the docker-shell.sh terminal, start it again and reattach VS Code" >&2
        exit 1
      fi
      MK="$MK BR_DIR=/br"
    fi
    echo "== $MK   (course container)"
    exec bash -c "$MK"
  fi
  if [ "$(uname -s)" = Linux ]; then
    if command -v docker >/dev/null 2>&1 && [ -n "$(docker images -q "${URS_IMAGE:-embedded-course}" 2>/dev/null)" ]; then
      echo "== $MK   (course container)"
      exec ./scripts/docker-shell.sh $MK
    fi
    echo "== $MK"
    exec bash -c "$MK"
  fi
  if command -v docker >/dev/null 2>&1; then
    echo "== $MK BR_DIR=/br   (course container)"
    exec ./scripts/docker-shell.sh bash -c "$MK BR_DIR=/br"
  fi
  echo "vscode-task.sh: Lab 7 needs Linux -- Docker or WSL2 (README, LV7)" >&2
  exit 1
fi

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
  build-vcd)   [ "$QEMU_SCRIPT" = run_qemu.sh ] || {
                 echo "vscode-task.sh: build-vcd is for labs without a network (LV1, LV2)" >&2
                 exit 1; }
               CMD="idf.py build && bash ../../common/run_qemu.sh --gpio-vcd build" ;;
  gdb)         CMD="bash ../../common/run_qemu_gdb.sh build" ;;
  build-gdb)   CMD="idf.py build && bash ../../common/run_qemu_gdb.sh build" ;;
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
  # The debugger runs where VS Code runs: with Docker that is the container
  # only when VS Code is attached to it (then idf.py is found above).
  if [ "$ACTION" = gdb ] || [ "$ACTION" = build-gdb ]; then
    echo "vscode-task.sh: with Docker, debug from VS Code attached to the course container (README, \"Rad u VS Codeu\")" >&2
    exit 1
  fi
  echo "== $CMD   (in $PROJ, course container)"
  cd "$ROOT"
  exec ./scripts/docker-shell.sh bash -c "cd $PROJ && $CMD"
fi

echo "vscode-task.sh: no ESP-IDF found (H1: path B, ~/esp-idf) and no docker (H1: path A)" >&2
exit 1
