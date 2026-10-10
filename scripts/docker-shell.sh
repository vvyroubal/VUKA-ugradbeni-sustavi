#!/usr/bin/env bash
# docker-shell.sh -- open a shell (or run one command) in the course container,
# with this repository mounted at /work. Builds the image on first use.
#
# Usage:
#     ./scripts/docker-shell.sh                      # interactive shell
#     ./scripts/docker-shell.sh bash scripts/verify.sh
#     URS_PORT=/dev/ttyACM0 ./scripts/docker-shell.sh   # Linux: pass the board in
#
# URS_PORT works on Linux hosts only: Docker Desktop on macOS and Windows does
# not pass USB devices to containers (see README, "Programiranje pločice").

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE="${URS_IMAGE:-embedded-course}"

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
  echo "== building image '$IMAGE' (first use; takes several minutes)"
  docker build -t "$IMAGE" "$ROOT"
fi

ARGS=(--rm -v "$ROOT:/work" -w /work -e IDF_GIT_SAFE_DIR=/work)
[ -t 0 ] && ARGS+=(-it)
[ -n "${URS_PORT:-}" ] && ARGS+=(--device "$URS_PORT")

if [ "$#" -eq 0 ]; then set -- bash; fi
exec docker run "${ARGS[@]}" "$IMAGE" "$@"
