#!/usr/bin/env bash
# docker-shell.sh -- open a shell (or run one command) in the course container,
# with this repository mounted at /work. Builds the image on first use.
#
# Bash 3.2 (macOS) safe: empty arrays are expanded with ${a[@]+"${a[@]}"}.
#
# Usage:
#     ./scripts/docker-shell.sh                      # interactive shell
#     ./scripts/docker-shell.sh bash scripts/verify.sh
#     URS_PORT=/dev/ttyACM0 ./scripts/docker-shell.sh   # Linux: pass the board in
#
# Second terminal: while a course container is running, running this script
# again attaches to THAT container (needed for GDB in Lab 2, and so the Lab 6
# broker and QEMU share one network). The container is named urs-toolchain
# and publishes port 8883 (the Lab 6 MQTT broker) to this computer.
#
# On Linux (and WSL2) the container runs as your own user, so files it
# creates in the repository belong to you, not to root.
#
# URS_PORT works on Linux hosts only: Docker Desktop on macOS and Windows does
# not pass USB devices to containers (see README, "Programiranje pločice").

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE="${URS_IMAGE:-embedded-course}"
NAME="${URS_CONTAINER:-urs-toolchain}"

if [ "$#" -eq 0 ]; then set -- bash; fi
TTY=(); [ -t 0 ] && TTY=(-it)

USER_ARGS=()
if [ "$(uname -s)" = Linux ]; then
  USER_ARGS=(--user "$(id -u):$(id -g)" -e HOME=/tmp)
fi

if [ -n "$(docker ps -q --filter "name=^/${NAME}\$")" ]; then
  echo "== attaching to the running '$NAME' container" >&2
  # the image's entrypoint activates ESP-IDF, as in the first terminal
  exec docker exec ${TTY[@]+"${TTY[@]}"} ${USER_ARGS[@]+"${USER_ARGS[@]}"} -w /work "$NAME" \
    /opt/esp/entrypoint.sh "$@"
fi

if [ -z "$(docker images -q "$IMAGE")" ]; then
  echo "== building image '$IMAGE' (first use: 15-30 min, ~8 GB)" >&2
  docker build -t "$IMAGE" "$ROOT"
fi

ARGS=(--rm --name "$NAME" -p 8883:8883 -v "$ROOT:/work" -w /work
      -e IDF_GIT_SAFE_DIR=/work)
ARGS+=(${USER_ARGS[@]+"${USER_ARGS[@]}"} ${TTY[@]+"${TTY[@]}"})
if [ -n "${URS_PORT:-}" ]; then
  ARGS+=(--device "$URS_PORT" --group-add "$(stat -c %g "$URS_PORT")")
fi

exec docker run "${ARGS[@]}" "$IMAGE" "$@"
