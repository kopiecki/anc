FROM debian:bullseye AS base

ENV GOSU_URL https://github.com/tianon/gosu/releases/download/1.12/gosu
COPY tools/docker/entrypoint.sh /usr/local/bin/entrypoint.sh

RUN apt-get update \
    && apt-get -y install \
        gnupg2 \
        ca-certificates \
        curl \
    && gpg --keyserver keys.openpgp.org --recv-keys B42F6819007F00F88E364FD4036A9C25BF357DD4 \
    && curl -o /usr/local/bin/gosu -SL "$GOSU_URL-$(dpkg --print-architecture)" \
    && curl -o /usr/local/bin/gosu.asc -SL "$GOSU_URL-$(dpkg --print-architecture).asc" \
    && gpg --verify /usr/local/bin/gosu.asc \
    && rm /usr/local/bin/gosu.asc \
    && rm -rf /var/lib/apt/lists/* \
    && chmod +x /usr/local/bin/gosu \
    && chmod +x /usr/local/bin/entrypoint.sh

RUN apt-get update \
    && apt-get install -y \
        sudo \
        apt-transport-https \
        software-properties-common \
        wget \
        git \
        lcov \
        xz-utils \
        unzip \
        bc \
        bison \
        flex \
        libgmp3-dev \
        libmpc-dev \
        libssl-dev \
        cpio \
        lzop \
        make \
        libc6-dev \
        libncurses5-dev \
        kmod \
        vim \
        quilt \
        gnutls-bin \
        build-essential \
    && apt-get clean

FROM base

RUN mkdir -p /opt/toolchain/ \
    && wget 'https://developer.arm.com/-/media/Files/downloads/gnu-a/8.2-2019.01/gcc-arm-8.2-2019.01-x86_64-arm-linux-gnueabihf.tar.xz' \
    && tar -C /opt/toolchain -xf gcc-arm-8.2-2019.01-x86_64-arm-linux-gnueabihf.tar.xz \
    && rm -rf gcc-arm-8.2-2019.01-x86_64-arm-linux-gnueabihf.tar.xz

RUN wget https://github.com/Kitware/CMake/releases/download/v3.19.2/cmake-3.19.2-Linux-x86_64.sh \
    && chmod +x cmake-3.19.2-Linux-x86_64.sh \
    && ./cmake-3.19.2-Linux-x86_64.sh --skip-license --prefix=/usr \
    && rm cmake-3.19.2-Linux-x86_64.sh

RUN wget -O - https://apt.llvm.org/llvm.sh > /tmp/clang_install.sh && bash /tmp/clang_install.sh 12 \
    && apt-get update && apt-get install -y clang-format-12 \
    && ln -s /usr/bin/clang-format-12 /usr/bin/clang-format

RUN chmod -R 777 /root

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

WORKDIR /opt/src
ENTRYPOINT [ "/usr/local/bin/entrypoint.sh" ]
CMD [ "bash" ]
