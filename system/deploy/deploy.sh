#!/usr/bin/bash

top=/mnt

case "$1" in
kernel)
    target=$top/kernel
    echo "Deploying kernel to $target..."
    sudo cp zImage am335x-boneblack.dtb $target
    ;;
uboot)
    target=$top/uboot
    echo "Deploying uboot to $target..."
    sudo cp uboot.env MLO u-boot.img $target
    ;;
*)
    echo "Bad specifier"
    ;;
esac
