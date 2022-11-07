PROJECT_NAME := anc

CORES := $(shell nproc)

ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR := build
TOOLCHAIN := cmake/toolchain/arm_linux.cmake

DOCKER_SERVICE_NAME := anc-software

DEBUG := y

#------------------------------------------------------------------------------

ifeq ($(DEBUG),y)
	CMAKE_CONF_FLAGS += -DCMAKE_BUILD_TYPE=Debug
else
	CMAKE_CONF_FLAGS += -DCMAKE_BUILD_TYPE=Release
endif

CMAKE_CONF_FLAGS += -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN)
CMAKE_CONF_FLAGS += -DPROJECT_NAME=$(PROJECT_NAME)

CMAKE := cmake
CMAKE_CONFIGURE := $(CMAKE) $(CMAKE_CONF_FLAGS) ..
CMAKE_BUILD := $(CMAKE) --build . $(CMAKE_BUILD_FLAGS) -- -j$(NCORES)

#------------------------------------------------------------------------------

default: build
.PHONY: default

#------------------------------------------------------------------------------

build: | .mk-build-dir
	@ cd $(BUILD_DIR) && \
	        [ -f CMakeCache.txt ] \
	            && echo "Build project $(PROJECT_NAME)" \
	            || $(CMAKE_CONFIGURE) && \
	        $(CMAKE_BUILD)
.PHONY: build

#------------------------------------------------------------------------------

.mk-build-dir:
	@ mkdir -p $(BUILD_DIR)
.PHONY: .mk-build-dir

#------------------------------------------------------------------------------

docker-run:
	@ docker-compose run --rm -e \
		LOCAL_USER_ID=$(shell id -u) \
		$(DOCKER_SERVICE_NAME) bash
.PHONY: docker-run

#------------------------------------------------------------------------------

docker-build:
	@ docker-compose build
.PHONY: docker-build

#------------------------------------------------------------------------------

clean:
	@ rm -rf "$(BUILD_DIR)"
.PHONY: clean

#------------------------------------------------------------------------------
