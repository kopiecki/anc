#!/usr/bin/bash

make build-kernel
make deploy-kernel
scp deploy/am335x-boneblack.dtb root@192.168.0.240:/mnt
