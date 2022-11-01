AT 						:= @
SHELL 					:= bash

DOCKER_SERVICE_NAME 	:= anc_linux
DOCKER_CONTAINER_NAME	:=

LINUX_DIR				:= /opt/linux
ARCH 					:= arm
CROSS_COMPILE			:= arm-linux-gnueabihf-

#------------------------------------------------------------------------------

docker: | .docker-build
	$(AT) docker-compose run -e LOCAL_USER_ID=$(id -u) $(DOCKER_SERVICE_NAME) $(SHELL)
.PHONY: docker

#------------------------------------------------------------------------------

linux:
	$(AT) cd $(LINUX_DIR) && \
		make
.PHONY: linux

#------------------------------------------------------------------------------

.docker-build:
	$(AT) docker-compose build
.PHONY: .docker-build

#------------------------------------------------------------------------------
