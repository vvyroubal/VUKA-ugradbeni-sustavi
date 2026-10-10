# Student development image for "Ugradbeni računalni sustavi" (VUKA):
# ESP-IDF v5.5 + Espressif QEMU (labs 1-6) + upstream QEMU and the Buildroot
# host tools (lab 7). No TeX Live -- this image only builds and runs code.
#
#   docker build -t embedded-course .
#
# The image name `embedded-course` is the one the lab manuals use.
FROM espressif/idf:v5.5

# qemu-system-misc: UPSTREAM qemu-system-riscv32 (/usr/bin) for lab 7's generic
#   'virt' board -- the Espressif QEMU fork cannot run it.
# gcc/g++/patch/cpio/unzip/rsync/bc/file/wget/make: Buildroot host dependencies.
# openssl: lab 6 certificate generation (broker/gen_certs.sh) and lab 7 signing.
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      make git python3 gcc g++ patch cpio unzip rsync bc file wget \
      qemu-system-misc openssl \
    && rm -rf /var/lib/apt/lists/*

# Espressif QEMU for the ESP32-C3 (riscv32). Tools live under $IDF_TOOLS_PATH;
# the symlink puts the fork first on PATH even outside the IDF export hook.
# It deliberately shadows the upstream /usr/bin/qemu-system-riscv32 -- lab 7's
# run_virt.sh reaches past it to the upstream binary explicitly.
RUN python3 $IDF_PATH/tools/idf_tools.py install qemu-riscv32 && \
    ln -sf "$(find $IDF_TOOLS_PATH/tools/qemu-riscv32 -type f -name qemu-system-riscv32 | head -1)" /usr/local/bin/qemu-system-riscv32 && \
    qemu-system-riscv32 --version

WORKDIR /work
