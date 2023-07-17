#!/usr/bin/bash

sd=/dev/sdc
sudo parted --script ${sd} mklabel msdos \
                      mkpart primary fat32 0% 100MiB \
                      mkpart primary ext4 100MiB 1100MiB \
                      mkpart primary ext4 1100MiB 100% \
                      set 1 boot on

sudo mkfs.vfat ${sd}1 -n uboot
sudo mkfs.ext4 ${sd}2 -L kernel
sudo mkfs.ext4 ${sd}3 -L rootfs
