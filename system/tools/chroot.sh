#!/usr/bin/bash

sudo cp /usr/bin/qemu-arm-static /mnt/rootfs/bin
sudo chroot /mnt/rootfs /bin/bash
