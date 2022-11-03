AT 								:= @
SHELL 							:= bash
NCORES							:= 2

DOCKER_SERVICE_NAME 			:= anc-linux
PATCHES_DIR						:= /opt/src/patches
MENUCONFIG						:= y

LINUX_REPO						:= https://github.com/beagleboard/linux.git
LINUX_TAG						:= 5.10.140-ti-rt-r52
LINUX_DIR						:= /opt/src/linux
LINUX_DEFAULT_CONFIG			:= bb.org_defconfig
LINUX_ARCH 						:= arm
LINUX_BUILD_OUTPUT				:= zImage modules dtbs
LINUX_DRIVERS_DIR				:= /opt/src/drivers

LINUX_TEMP_DRIVERS_DIR			:= /opt/src/drivers
LINUX_TEMP_BINDINGS_DIR			:= /opt/src/bindings

TOOLCHAIN						:= /opt/toolchain/gcc-arm-8.2-2019.01-x86_64-arm-linux-gnueabihf/bin
CROSS_COMPILER					:= $(TOOLCHAIN)/arm-linux-gnueabihf-

AD400X_PATCH					:= $(PATCHES_DIR)/ad400x_driver.patch

#------------------------------------------------------------------------------

ifeq ($(LINUX_CONFIG),)
ifeq ($(MENUCONFIG),y)
LINUX_CONFIG += menuconfig
else
LINUX_CONFIG += $(LINUX_DEFAULT_CONFIG)
endif
endif

#------------------------------------------------------------------------------

docker-run:
	$(AT) docker-compose run --rm -e \
		LOCAL_USER_ID=$(shell id -u) \
		$(DOCKER_SERVICE_NAME) $(SHELL)
.PHONY: docker-run

#------------------------------------------------------------------------------

docker-build:
	$(AT) docker-compose build
.PHONY: docker-build

#------------------------------------------------------------------------------

linux: | .linux-configure
	$(AT) cd $(LINUX_DIR) && \
	echo "Output: $(LINUX_BUILD_OUTPUT)" && \
	make \
		ARCH=$(LINUX_ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILER) \
		$(LINUX_BUILD_OUTPUT) 
.PHONY: linux

#------------------------------------------------------------------------------

.linux-configure:
	$(AT) cd $(LINUX_DIR) && \
		echo "Config: $(LINUX_CONFIG)" && \
		make \
			ARCH=$(LINUX_ARCH) \
			CROSS_COMPILE=$(CROSS_COMPILER) \
			$(LINUX_CONFIG)
.PHONY: .linux-configure

#------------------------------------------------------------------------------

linux-fetch:
	$(AT) git clone --depth 1 --branch $(LINUX_TAG) $(LINUX_REPO)
.PHONY: linux-fetch

#------------------------------------------------------------------------------

linux-apply-patches:
	$(AT) patch -s -p0 < $(AD400X_PATCH)
.PHONY: linux-apply-patches

#------------------------------------------------------------------------------

clean:
	$(AT) rm -rf $(LINUX_DIR)
.PHONY: clean

#------------------------------------------------------------------------------
