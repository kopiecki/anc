#!/usr/bin/bash

sd=/dev/mmcblk0
sudo parted --script ${sd} mklabel msdos \
                      mkpart primary fat32 0% 100MiB \
                      mkpart primary ext4 100MiB 1100MiB \
                      mkpart primary ext4 1100MiB 100% \
                      set 1 boot on

sudo mkfs.vfat ${sd}p1 -n uboot
sudo mkfs.ext4 ${sd}p2 -L kernel
sudo mkfs.ext4 ${sd}p3 -L rootfs
