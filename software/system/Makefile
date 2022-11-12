NCORES := 4

TARGET ?=
T ?= 

SYSTEM_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

LINUX_DIR := $(SYSTEM_DIR)/linux
UBOOT_DIR := $(SYSTEM_DIR)/u-boot
ROOTFS_DIR := $(SYSTEM_DIR)/rootfs
DEPLOY_DIR := $(SYSTEM_DIR)/deploy

LINUX_REPO := https://github.com/beagleboard/linux.git
UBOOT_REPO := https://github.com/u-boot/u-boot.git

ROOTFS_HTTPS := https://rcn-ee.com/rootfs/eewiki/minfs/debian-11.5-minimal-armhf-2022-10-06.tar.xz
ROOTFS_TAR := debian-11.5-minimal-armhf-2022-10-06.tar.xz

UBOOT_DEFAULT_CONFIG := am335x_evm_defconfig
LINUX_DEFAULT_CONFIG := bb.org_defconfig

LINUX_TAG := 5.10.140-ti-rt-r52
UBOOT_TAG := v2023.01-rc1

DEVICE_TREE := am335x-boneblack.dtb

LINUX_BUILD_OUTPUT :=

ARCH := arm
TOOLCHAIN := /opt/toolchain/gcc-arm-8.2-2019.01-x86_64-arm-linux-gnueabihf/bin
CROSS_COMPILER := $(TOOLCHAIN)/arm-linux-gnueabihf-

#------------------------------------------------------------------------------

ifndef TARGET
$(error TARGET not specified)
else
	T := $(shell echo $(TARGET) | tr a-z A-Z)
	DIR := $($(T)_DIR)
	REPO := $($(T)_REPO)
	TAG := $($(T)_TAG)
	CONFIG := $($(T)_DEFAULT_CONFIG)
	OUTPUT := $($(T)_OUTPUT)
endif

#------------------------------------------------------------------------------

default: build
.PHONY: default

#------------------------------------------------------------------------------

build:
ifeq ($(TARGET), rootfs)
	$(error Using premade rootfs)
else
	@ cd $(DIR) && \
		make \
			ARCH=$(ARCH) \
			CROSS_COMPILE=$(CROSS_COMPILER) \
			$(OUTPUT) \
			-j$(NCORES)
endif
.PHONY: build

#------------------------------------------------------------------------------

menuconfig:
ifeq ($(TARGET), rootfs)
	$(error Using premade rootfs)
else
	@ cd $(DIR) && \
	make \
		ARCH=$(ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILER) \
		menuconfig \
		-j$(NCORES)
endif
.PHONY: menuconfig

#------------------------------------------------------------------------------

configure:
ifeq ($(TARGET), rootfs)
	$(error Using premade rootfs)
else
	@ cd $(DIR) && \
		make \
			ARCH=$(ARCH) \
			CROSS_COMPILE=$(CROSS_COMPILER) \
			$(CONFIG) \
			-j$(NCORES)
endif
.PHONY: configure

#------------------------------------------------------------------------------

fetch:
ifeq ($(TARGET), rootfs)
	@ mkdir -p $(DIR) && \
	cd $(DIR) && \
	wget $(ROOTFS_HTTPS) && \
	tar -xf $(ROOTFS_TAR) && \
	rm $(ROOTFS_TAR)
else
	@ git clone --depth 1 --branch $(TAG) $(REPO)
endif
.PHONY: fetch

#------------------------------------------------------------------------------

clean:
	@ rm -rf $(DIR)
.PHONY: clean

#------------------------------------------------------------------------------
