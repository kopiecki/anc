PROJECT_NAME := anc
FUNCTIONAL_TEST_NAME := functional

CORES := $(shell nproc)

ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR := build
TOOLCHAIN := native

DEBUG := y
TEST_PLATFORM := dkop@192.168.0.228

#------------------------------------------------------------------------------

ifeq ($(DEBUG),y)
	CMAKE_CONF_FLAGS += -DCMAKE_BUILD_TYPE=Debug
else
	CMAKE_CONF_FLAGS += -DCMAKE_BUILD_TYPE=Release
endif

CMAKE_CONF_FLAGS += -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/$(TOOLCHAIN).cmake
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

run_app: 
	@ ./$(BUILD_DIR)/software/app/anc/$(PROJECT_NAME)
.PHONY: run

#------------------------------------------------------------------------------

run_unit_test:
	@ cd test/unit && ceedling
.PHONY: run_unit_test

#------------------------------------------------------------------------------

run_test_fir: 
	@ ./$(BUILD_DIR)/test/test_fir
.PHONY: run_test_fir

#------------------------------------------------------------------------------

run_test_lms: 
	@ ./$(BUILD_DIR)/test/test_lms
.PHONY: run_test_lms

#------------------------------------------------------------------------------

run_test_signal: 
	@ ./$(BUILD_DIR)/test/test_signal
.PHONY: run_test_signal

#------------------------------------------------------------------------------

.mk-build-dir:
	@ mkdir -p $(BUILD_DIR)
.PHONY: .mk-build-dir

#------------------------------------------------------------------------------

deploy_sound_test:
	@ scp $(BUILD_DIR)/test/test_sound $(TEST_PLATFORM):/home/dkop/anc
.PHONY: deploy_sound_test

#------------------------------------------------------------------------------

deploy_anc_test:
	@ scp $(BUILD_DIR)/test/test_anc $(TEST_PLATFORM):/home/dkop/anc
.PHONY: deploy_anc_test

#------------------------------------------------------------------------------

get_captured_samples:
	@scp $(TEST_PLATFORM):/home/dkop/anc/capture.csv .
.PHONY: get_captured_samples

#------------------------------------------------------------------------------

clean:
	@ rm -rf "$(BUILD_DIR)"
.PHONY: clean
