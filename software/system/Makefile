ncores := 4

system_dir := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

linux_dir := $(system_dir)/linux
linux_repo := https://github.com/beagleboard/linux.git
linux_tag := 6.1.26-ti-rt-r4
linux_config := bb.org_defconfig
device_tree := am335x-boneblack.dtb
modules_path := /mnt/rootfs
zimage_path := $(linux_dir)/arch/arm/boot/zImage
dtb_path := $(linux_dir)/arch/arm/boot/dts/$(device_tree)

uboot_dir := $(system_dir)/u-boot
uboot_repo := https://github.com/u-boot/u-boot.git
uboot_tag := v2023.04
uboot_config := am335x_evm_defconfig
mlo_path := $(uboot_dir)/MLO
uboot_img_path := $(uboot_dir)/u-boot.img

deploy_dir := $(system_dir)/deploy

arch := arm
toolchain := arm-linux-gnueabihf
cross_compile := arm-linux-gnueabihf-

#------------------------------------------------------------------------------

kernel: fetch-kernel configure-kernel build-kernel deploy-kernel install-modules
.PHONY: kernel

#------------------------------------------------------------------------------

uboot: fetch-uboot configure-uboot build-uboot deploy-uboot
.PHONY: uboot

#------------------------------------------------------------------------------

fetch-kernel:
	@ git clone --depth 1 --branch $(linux_tag) $(linux_repo) && \
		echo "Kernel fetch [tag: $(linux_tag), repo: $(linux_repo)] to $(linux_dir)"
.PHONY: fetch

#------------------------------------------------------------------------------

configure-kernel:
	@ cd $(linux_dir) && \
		make \
			ARCH=$(arch) \
			CROSS_COMPILE=$(cross_compile) \
			$(linux_config) \
			-j$(ncores) && \
				echo "Configured kernel"
.PHONY: configure

#------------------------------------------------------------------------------

build-kernel:
	@ cd $(linux_dir) && \
		make \
			ARCH=$(arch) \
			CROSS_COMPILE=$(cross_compile) \
			$(output) \
			-j$(ncores) && \
				echo "Build kernel"
.PHONY: build-kernel

#------------------------------------------------------------------------------

install-modules:
	@ cd $(linux_dir) && \
		make \
			ARCH=$(arch) \
			CROSS_COMPILE=$(cross_compile) \
			INSTALL_MOD_PATH=$(modules_path) modules_install && \
				echo "Modules installed to $(modules_path)."
.PHONY: install-modules

#------------------------------------------------------------------------------

deploy-kernel:
	@ cd $(linux_dir) && \
		cp -r $(zimage_path) $(dtb_path) $(deploy_dir) && \
			echo "Deployed kernel to $(deploy_dir)"
.PHONY: deploy_kernel

#------------------------------------------------------------------------------

kernel-menuconfig:
	@ cd $(linux_dir) && \
		make \
			ARCH=$(arch) \
			CROSS_COMPILE=$(cross_compile) \
			menuconfig \	
			-j$(ncores) && \
				"Configured kernel"
.PHONY: menuconfig

#------------------------------------------------------------------------------

fetch-uboot:
	@ git clone --depth 1 --branch $(uboot_tag) $(uboot_repo) && \
		echo "Fetched u-boot [tag: $(uboot_tag), repo: $(uboot_repo)] to $(uboot_dir)"
.PHONY: fetch

#------------------------------------------------------------------------------

configure-uboot:
	@ cd $(uboot_dir) && \
		make \
			ARCH=$(arch) \
			CROSS_COMPILE=$(cross_compile) \
			$(uboot_config) \
			-j$(ncores) && \
				echo "Configured u-boot"
.PHONY: configure

#------------------------------------------------------------------------------

build-uboot:
	@ cd $(uboot_dir) && \
		make \
			ARCH=$(arch) \
			CROSS_COMPILE=$(cross_compile) \
			$(output) \
			-j$(ncores) && \
				echo "Build u-boot"
.PHONY: build-uboot

#------------------------------------------------------------------------------

deploy-uboot:
	@ cd $(uboot_dir) && \
		cp -r $(uboot_img_path) $(mlo_path) $(deploy_dir) && \
			echo "Deployed u-boot to $(deploy_dir)"
.PHONY: deploy_kernel

#------------------------------------------------------------------------------

clean:
	@ rm -rf $(DIR)
.PHONY: clean

#------------------------------------------------------------------------------
