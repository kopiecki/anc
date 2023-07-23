# syntax=docker/dockerfile:1.4

ARG BASE_IMG=ubuntu:22.04
FROM ${BASE_IMG}

ARG GCC_VERSION=12
ARG CLANG_FORMAT_VERSION=14

# basic packages/deps
RUN --mount=type=bind,source=base/requirements.txt,dst=/tmp/requirements.txt \
    --mount=type=bind,source=base/gitconfig,dst=/tmp/gitconfig \
    apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        apt-transport-https \
        ca-certificates \
        software-properties-common \
        wget \
    && dpkg --add-architecture i386 \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        bash-completion \
        curl \
        git \
        git-lfs \
        git-flow \
        gosu \ 
        sudo \
        vim \
        net-tools \
        iputils-ping \
        cmake \
        cmake-data \
        g++-$GCC_VERSION \
        gcc-$GCC_VERSION \
        gdb \
        gdb-multiarch \
        ninja-build \
        # u-boot deps
        openssl \
        libssl-dev \
        # kernel deps
        libncurses-dev \
        libgmp3-dev \
        libmpc-dev \
        gawk \
        flex \
        bison \
        bc \
        mpc \
        dkms \
        cpio \
        autoconf \
        liblz4-tool \
        # rootfs
        debootstrap \
        schroot \
        # python
        python3 \
        python3-distutils \
        python3-pip \
        python3-venv \
        # package
        rpm \
        clang-format-$CLANG_FORMAT_VERSION \
        lcov \
        # docs
        doxygen \
        imagemagick \
        graphviz \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/gcc gcc \
        /usr/bin/gcc-$GCC_VERSION 100 --slave /usr/bin/g++ g++ \
        /usr/bin/g++-$GCC_VERSION --slave /usr/bin/gcov gcov /usr/bin/gcov-$GCC_VERSION \
    && ln -s /usr/bin/clang-format-$CLANG_FORMAT_VERSION /usr/bin/clang-format \
    && python3 -m pip install -U pip \
    && cat /tmp/gitconfig >> /etc/gitconfig


# ARM toolchain
ARG TARGET_ARCH=arm-none-linux-gnueabihf
ARG HOST_ARCH=x86_64
ARG TOOLCHAIN_VER=10.3-2021.07
ARG ARM_LINK=https://developer.arm.com/-/media/Files/downloads/gnu-a
ARG TOOLCHAIN_TARBALL=gcc-arm-${TOOLCHAIN_VER}-${HOST_ARCH}-${TARGET_ARCH}.tar.xz
ARG TOOLCHAIN_LINK=${ARM_LINK}/${TOOLCHAIN_VER}/binrel/${TOOLCHAIN_TARBALL}
ARG ARM_TOOLCHAIN_PATH=/opt/${TARGET_ARCH}

RUN mkdir -p ${ARM_TOOLCHAIN_PATH} \
    && wget ${TOOLCHAIN_LINK} \
    && tar xf ${TOOLCHAIN_TARBALL} -C ${ARM_TOOLCHAIN_PATH} --strip-components 1 \
    && rm -rf ${TOOLCHAIN_TARBALL}

ENV ARM_TOOLCHAIN_PATH=${ARM_TOOLCHAIN_PATH} 
ENV PATH="${PATH}:${ARM_TOOLCHAIN_PATH}/bin"

# ALSA library
ARG ALSA_LIB_VER=1.2.7.2
ARG ALSA_LIB_TARBALL=alsa-lib-${ALSA_LIB_VER}.tar.bz2
ARG ALSA_LIB_LINK=https://www.alsa-project.org/files/pub/lib/${ALSA_LIB_TARBALL}
ARG ALSA_LIB_PATH=/opt/alsa-lib

RUN mkdir ${ALSA_LIB_PATH} \
    && wget ${ALSA_LIB_LINK} \
    && tar xf ${ALSA_LIB_TARBALL} -C ${ALSA_LIB_PATH} --strip-components 1 \
    && rm -rf ${ALSA_LIB_TARBALL} \
    && cd ${ALSA_LIB_PATH} \
    && ./configure --enable-shared=yes --enable-static=no --with-pic \
        --host=${TARGET_ARCH} --prefix=${ARM_TOOLCHAIN_PATH} \
    && make -j${nproc} \
    && make install

ENV ALSA_LIB_PATH=${ALSA_LIB_PATH}

# environment setup
COPY --link \
     base/entrypoint.sh \
     base/scripts/* \
         /usr/local/bin/

ENV LC_ALL=C.UTF-8 \
    LANG=C.UTF-8

WORKDIR /opt/src
RUN mkdir -p /docker/entrypoint.d/ \
    && touch /docker/entrypoint.d/00-dummy.sh \
    && chmod +x /docker/entrypoint.d/00-dummy.sh
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["/bin/bash"]

