#!/usr/bin/env bash
# make_bundle.sh — build a signed A/B OTA bundle for Lab 7 (host side).
#
# Produces a tar bundle { rootfs.ext4, rootfs.ext4.sig, version } where:
#   * rootfs.ext4     is a copy of a base image with /etc/lab7-release rewritten
#                     to the requested version + slot (edited offline via debugfs,
#                     no mount / no root needed),
#   * rootfs.ext4.sig is an RSA-SHA256 signature over that exact image,
#   * version         is the human-readable version string.
#
# The signature is what the target verifies (openssl dgst -sha256 -verify) against
# the public key baked into its rootfs. Sign the IMAGE, not the download channel.
#
# Usage:
#   make_bundle.sh --in BASE.ext4 --version X.Y.Z --slot a|b \
#                  [--key PRIV.pem] [--out BUNDLE.tar]
#
# Defaults: --key ota/keys/demo_signing_key.INSECURE.pem  (the throwaway teaching
# key; a real product keeps its private key offline / in an HSM and never ships
# or commits it), --out artifacts/ota_vVERSION_slotSLOT.tar.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

IN="" VERSION="" SLOT="" KEY="$SCRIPT_DIR/keys/demo_signing_key.INSECURE.pem" OUT=""
while [ $# -gt 0 ]; do
  case "$1" in
    --in)      IN="$2"; shift 2 ;;
    --version) VERSION="$2"; shift 2 ;;
    --slot)    SLOT="$2"; shift 2 ;;
    --key)     KEY="$2"; shift 2 ;;
    --out)     OUT="$2"; shift 2 ;;
    -h|--help) sed -n '2,20p' "$0"; exit 0 ;;
    *) echo "make_bundle.sh: unknown arg '$1'" >&2; exit 2 ;;
  esac
done

[ -n "$IN" ] && [ -n "$VERSION" ] && [ -n "$SLOT" ] \
  || { echo "make_bundle.sh: --in, --version and --slot are required" >&2; exit 2; }
[ -f "$IN" ]  || { echo "make_bundle.sh: base image '$IN' not found" >&2; exit 2; }
[ -f "$KEY" ] || { echo "make_bundle.sh: signing key '$KEY' not found" >&2; exit 2; }
case "$SLOT" in a|b) ;; *) echo "make_bundle.sh: --slot must be a or b" >&2; exit 2 ;; esac

SLOT_UP=$(printf '%s' "$SLOT" | tr 'a-z' 'A-Z')
OUT="${OUT:-$LAB_DIR/artifacts/ota_v${VERSION}_slot${SLOT}.tar}"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# 1. copy the base image and rewrite its version marker offline (debugfs).
cp "$IN" "$WORK/rootfs.ext4"
printf 'LAB7_VERSION=%s\nLAB7_SLOT=%s\n' "$VERSION" "$SLOT_UP" > "$WORK/lab7-release"
debugfs -w "$WORK/rootfs.ext4" >/dev/null 2>&1 <<EOF
rm /etc/lab7-release
write $WORK/lab7-release /etc/lab7-release
quit
EOF

# 2. sign the image, 3. record the version, 4. pack the tar.
openssl dgst -sha256 -sign "$KEY" -out "$WORK/rootfs.ext4.sig" "$WORK/rootfs.ext4"
printf '%s\n' "$VERSION" > "$WORK/version"
tar -C "$WORK" -cf "$OUT" rootfs.ext4 version rootfs.ext4.sig

echo "make_bundle.sh: wrote $OUT"
echo "  version=$VERSION slot=$SLOT_UP  signed with $(basename "$KEY")"
