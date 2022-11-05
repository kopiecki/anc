AT := @
SHELL := bash
NCORES := $(shell nproc)

ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

DOCKER_SERVICE_NAME := anc-software

SYSTEM_DIR := $(ROOT_DIR)/system
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

docker:
	$(AT) docker-compose run --rm -e \
		LOCAL_USER_ID=$(shell id -u) \
		$(DOCKER_SERVICE_NAME) $(SHELL)
.PHONY: docker

#------------------------------------------------------------------------------

docker-build:
	$(AT) docker-compose build
.PHONY: docker-build

#------------------------------------------------------------------------------

system: | .system-menuconfig
	$(AT) cd $(SYSTEM_DIR) && \
	make \
		ARCH=$(SYSTEM_ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILER) \
		$(SYSTEM_BUILD_OUTPUT) \
		-j$(NCORES)
.PHONY: system

#------------------------------------------------------------------------------

.system-menuconfig: | .system-configure
ifeq ($(SYSTEM_MENUCONFIG),y)
	$(AT) cd $(SYSTEM_DIR) && \
	make \
		ARCH=$(SYSTEM_ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILER) \
		menuconfig \
		-j$(NCORES)
else
	$(AT) echo "Omitting menuconfig"
endif
.PHONY: .system-menuconfig

#------------------------------------------------------------------------------

.system-configure: | .system-fetch
ifeq ($(SYSTEM_CONFIGURE),y)
	$(AT) echo "Configuring with $(SYSTEM_CONFIG)" && \
	cd $(SYSTEM_DIR) && \
		echo "Config: $(SYSTEM_CONFIG)" && \
		make \
			ARCH=$(SYSTEM_ARCH) \
			CROSS_COMPILE=$(CROSS_COMPILER) \
			$(SYSTEM_CONFIG)
else
	$(AT) echo "Copying config file from $(SYSTEM_CONFIG_DIR)" && \
	cp -r $(SYSTEM_CONFIG_DIR)/.config $(SYSTEM_DIR)
endif
.PHONY: .system-configure

#------------------------------------------------------------------------------

.system-fetch:
	$(AT) if [ ! -d "$(SYSTEM_LINUX_DIR)" ]; then \
		echo "Fetching from $(SYSTEM_REPO), tag: $(SYSTEM_TAG)" && \
		git clone --depth 1 --branch $(SYSTEM_TAG) $(SYSTEM_REPO); \
	fi
.PHONY: .system-fetch

#------------------------------------------------------------------------------

clean-system:
	$(AT) rm -rf $(SYSTEM_DIR)
.PHONY: clean-system

#------------------------------------------------------------------------------
