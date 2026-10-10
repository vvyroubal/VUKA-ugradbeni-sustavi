# Lab 7 BR2_EXTERNAL tree — no custom packages yet.
#
# Buildroot includes this file for every BR2_EXTERNAL tree. It is intentionally
# empty: Lab 7's rootfs additions ride in via the rootfs overlay (see
# BR2_ROOTFS_OVERLAY in configs/lab7_defconfig), not as Buildroot packages.
# Package .mk includes would go here (e.g. $(sort $(wildcard $(BR2_EXTERNAL_LAB7_PATH)/package/*/*.mk))).
