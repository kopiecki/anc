CORES := $(shell nproc)

SYSTEM_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SYSTEM_PATCHES_DIR := $(SYSTEM_DIR)/patches
SYSTEM_CONFIG_DIR := $(SYSTEM_DIR)/config
SYSTEM_LINUX_DIR := $(SYSTEM_DIR)/linux

SYSTEM_REPO := https://github.com/beagleboard/linux.git
SYSTEM_TAG := 5.10.140-ti-rt-r52
SYSTEM_MENUCONFIG := n
SYSTEM_CONFIGURE := n
SYSTEM_DEFAULT_CONFIG := bb.org_defconfig
SYSTEM_ARCH := arm
SYSTEM_BUILD_OUTPUT := zImage modules dtbs

TOOLCHAIN := /opt/toolchain/gcc-arm-8.2-2019.01-x86_64-arm-linux-gnueabihf/bin
CROSS_COMPILER := $(TOOLCHAIN)/arm-linux-gnueabihf-

#------------------------------------------------------------------------------

default: build
.PHONY: default

#------------------------------------------------------------------------------

build: | .menuconfig
	@ cd $(SYSTEM_LINUX_DIR) && \
	make \
		ARCH=$(SYSTEM_ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILER) \
		$(SYSTEM_BUILD_OUTPUT) \
		-j$(NCORES)
.PHONY: build

#------------------------------------------------------------------------------

.menuconfig: | .configure
ifeq ($(SYSTEM_MENUCONFIG),y)
	@ cd $(SYSTEM_LINUX_DIR) && \
	make \
		ARCH=$(SYSTEM_ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILER) \
		menuconfig \
		-j$(NCORES)
else
	@ echo "Omitting menuconfig"
endif
.PHONY: .menuconfig

#------------------------------------------------------------------------------

.configure: | .fetch
ifeq ($(SYSTEM_CONFIGURE),y)
	@ echo "Configuring with $(SYSTEM_CONFIG)" && \
	cd $(SYSTEM_LINUX_DIR) && \
		echo "Config: $(SYSTEM_CONFIG)" && \
		make \
			ARCH=$(SYSTEM_ARCH) \
			CROSS_COMPILE=$(CROSS_COMPILER) \
			$(SYSTEM_CONFIG)
else
	@ echo "Copying config file from $(SYSTEM_CONFIG_DIR)" && \
	cp -r $(SYSTEM_CONFIG_DIR)/.config $(SYSTEM_LINUX_DIR)
endif
.PHONY: .configure

#------------------------------------------------------------------------------

.fetch:
	@ if [ ! -d "$(SYSTEM_LINUX_DIR)" ]; then \
		echo "Fetching from $(SYSTEM_REPO), tag: $(SYSTEM_TAG)" && \
		git clone --depth 1 --branch $(SYSTEM_TAG) $(SYSTEM_REPO); \
	fi
.PHONY: .fetch

#------------------------------------------------------------------------------

clean:
	@ rm -rf $(SYSTEM_LINUX_DIR)
.PHONY: clean

#------------------------------------------------------------------------------
