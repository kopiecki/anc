#!/usr/bin/bash

target=/mnt/rootfs
arch=armhf
dist=stable
https=https://deb.debian.org/debian/
include="vim,ntp,ssh,libasound2,alsa-tools,alsa-utils,alsa-oss"

sudo debootstrap --arch=$arch --include=$include $dist $target $https
