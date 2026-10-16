#!/usr/bin/env bash
# gen_certs.sh — generate the self-signed TLS material for Lab 6's MQTT broker.
#
# Usage: bash labs/lab6_mqtt/broker/gen_certs.sh
#
# Produces, in this directory's certs/ (all .gitignore'd — NEVER committed;
# regenerate per run):
#   ca.key      CA private key
#   ca.crt      self-signed CA certificate (10-year)
#   server.key  broker private key
#   server.crt  broker certificate, signed by the CA, with
#               CN=10.0.2.2 and subjectAltName = IP:10.0.2.2, IP:127.0.0.1
#               (10.0.2.2 is the QEMU user-mode host alias the guest connects
#                to; 127.0.0.1 lets you smoke the broker from the host too).
#
# The CA cert is also copied to the starter/ and solution/ projects at
# main/certs/ca.crt, where each project's CMakeLists embeds it into the firmware
# (EMBED_TXTFILES) so the device can verify the broker. Those copies are
# .gitignore'd as well.
#
# Requires: openssl (host or the embedded-course container both have it).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CERT_DIR="$SCRIPT_DIR/certs"
DAYS=3650

mkdir -p "$CERT_DIR"

echo "== gen_certs.sh: CA =="
openssl genrsa -out "$CERT_DIR/ca.key" 2048
openssl req -x509 -new -nodes -key "$CERT_DIR/ca.key" -sha256 -days "$DAYS" \
    -subj "/C=HR/O=VUKA/OU=Ugradbeni racunalni sustavi/CN=VUKA Lab6 CA" \
    -out "$CERT_DIR/ca.crt"

echo "== gen_certs.sh: server cert (CN=10.0.2.2, SAN IP:10.0.2.2,127.0.0.1) =="
openssl genrsa -out "$CERT_DIR/server.key" 2048
openssl req -new -key "$CERT_DIR/server.key" \
    -subj "/C=HR/O=VUKA/OU=Ugradbeni racunalni sustavi/CN=10.0.2.2" \
    -out "$CERT_DIR/server.csr"
openssl x509 -req -in "$CERT_DIR/server.csr" \
    -CA "$CERT_DIR/ca.crt" -CAkey "$CERT_DIR/ca.key" -CAcreateserial \
    -days "$DAYS" -sha256 \
    -extfile <(printf 'subjectAltName=IP:10.0.2.2,IP:127.0.0.1\n') \
    -out "$CERT_DIR/server.crt"
rm -f "$CERT_DIR/server.csr" "$CERT_DIR/ca.srl"

echo "== gen_certs.sh: distributing ca.crt to projects =="
for proj in starter solution; do
    [ -d "$LAB_DIR/$proj/main" ] || continue   # students have no solution/
    mkdir -p "$LAB_DIR/$proj/main/certs"
    cp "$CERT_DIR/ca.crt" "$LAB_DIR/$proj/main/certs/ca.crt"
    echo "  -> $proj/main/certs/ca.crt"
done

# The broker runs in a container as its own user (mosquitto, uid 1883), so
# on a Linux host it cannot read a 0600 key owned by you. These are
# throw-away demo certificates for a local lab broker -- never do this with
# a real server key.
chmod 644 "$CERT_DIR/server.key"

echo "== gen_certs.sh: done. Certs in $CERT_DIR (not committed) =="
