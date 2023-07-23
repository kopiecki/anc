#!/usr/bin/bash

sd=/dev/sdc

sudo mkdir -p /mnt/uboot
sudo mkdir -p /mnt/kernel
sudo mkdir -p /mnt/rootfs

echo "Mounting uboot..."
sudo mount ${sd}1 /mnt/uboot
echo "Mounting kernel..."
sudo mount ${sd}2 /mnt/kernel
echo "Mounting rootfs..."
sudo mount ${sd}3 /mnt/rootfs

sync
