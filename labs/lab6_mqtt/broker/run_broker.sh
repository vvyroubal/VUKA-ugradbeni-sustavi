#!/usr/bin/env bash
# run_broker.sh — start the Lab 6 MQTT broker (Mosquitto, TLS on :8883) in Docker.
#
# Usage: bash labs/lab6_mqtt/broker/run_broker.sh
#
# Prerequisites:
#   - Docker running on the host.
#   - Certs generated first:  bash labs/lab6_mqtt/broker/gen_certs.sh
#
# Runs the official eclipse-mosquitto image in the foreground (Ctrl-C to stop),
# publishing container :8883 to host :8883, with this directory's mosquitto.conf
# and certs/ mounted read-only. The device (real or in QEMU) then connects to
# mqtts://10.0.2.2:8883 — under QEMU user-mode networking 10.0.2.2 is the host,
# so the guest reaches this broker; on a real board point it at the host's LAN
# IP and regenerate the server cert with that IP in the SAN.
#
# This is the interactive/manual broker for the real TLS lab. The automated
# qemu_smoke.sh does NOT use this (it cannot rely on Docker being available
# inside the build container) — it runs a tiny plaintext broker instead.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CERT_DIR="$SCRIPT_DIR/certs"
IMAGE="eclipse-mosquitto:2"

if [ ! -f "$CERT_DIR/server.crt" ]; then
  echo "run_broker.sh: no certs found — run 'bash $SCRIPT_DIR/gen_certs.sh' first" >&2
  exit 1
fi

echo "== run_broker.sh: starting $IMAGE with TLS on :8883 (Ctrl-C to stop) =="
exec docker run --rm -it \
  -p 8883:8883 \
  -v "$SCRIPT_DIR/mosquitto.conf:/mosquitto/config/mosquitto.conf:ro" \
  -v "$CERT_DIR:/mosquitto/certs:ro" \
  "$IMAGE"
