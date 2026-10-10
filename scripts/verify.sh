#!/usr/bin/env bash
# verify.sh -- check that the toolchain works end to end.
#
# Usage (from the repository root, with the ESP-IDF environment active --
# inside the Docker container it already is):
#     bash scripts/verify.sh
#
# Steps: (1) idf.py is on PATH and reports ESP-IDF v5.5; (2) the Espressif
# qemu-system-riscv32 is on PATH; (3) ESP-IDF's hello_world builds for the
# esp32c3 target; (4) the result boots in QEMU and prints "Hello world!".
# Exits 0 only if all four succeed.

set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${TMPDIR:-/tmp}/urs-verify-hello_world"
fail() { echo "verify.sh: FAIL -- $*" >&2; exit 1; }

echo "== 1/4 idf.py"
command -v idf.py >/dev/null 2>&1 \
  || fail "idf.py not on PATH (activate ESP-IDF: '. \$HOME/esp-idf/export.sh')"
VERSION="$(idf.py --version 2>/dev/null | tail -1)"
echo "   $VERSION"
case "$VERSION" in
  *v5.5*) ;;
  *) fail "expected ESP-IDF v5.5, got: $VERSION" ;;
esac
[ -n "${IDF_PATH:-}" ] || fail "IDF_PATH is not set"

echo "== 2/4 qemu-system-riscv32"
command -v qemu-system-riscv32 >/dev/null 2>&1 \
  || fail "qemu-system-riscv32 not on PATH (run: python3 \$IDF_PATH/tools/idf_tools.py install qemu-riscv32, then re-activate)"
qemu-system-riscv32 -M help 2>/dev/null | grep -q esp32c3 \
  || fail "this qemu-system-riscv32 has no esp32c3 machine (not the Espressif build)"
echo "   $(command -v qemu-system-riscv32)"

echo "== 3/4 build hello_world for esp32c3 (first build takes a few minutes)"
idf.py -C "$IDF_PATH/examples/get-started/hello_world" -B "$BUILD_DIR" \
    set-target esp32c3 build >"$BUILD_DIR.log" 2>&1 \
  || { tail -20 "$BUILD_DIR.log" >&2; fail "build failed (full log: $BUILD_DIR.log)"; }

echo "== 4/4 boot in QEMU"
bash "$ROOT/labs/common/run_qemu.sh" "$BUILD_DIR" 20 >/dev/null 2>&1 \
  || fail "QEMU run failed (see $BUILD_DIR/qemu.log)"
grep -q "Hello world!" "$BUILD_DIR/qemu.log" \
  || fail "'Hello world!' not found in $BUILD_DIR/qemu.log"

echo "verify.sh: PASS -- toolchain, build and QEMU all work"
