FROM debian:bullseye AS arm_base

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
 
RUN apt-get update  \
    && apt-get install -y \
        apt-transport-https \
        software-properties-common wget

RUN apt-get update  \
    && apt-get install -y \
        git \
        git-lfs \
        ruby \
        cmake \
        ninja-build \
        clang-format-9 \
        clang-tidy \
        lcov \
        python3-pip \
        python3-setuptools \
        sudo \
        gcc-arm-linux-gnueabihf \
        bison \
        flex \
    && gem install ceedling -v 0.30 \
    && ln -s /usr/bin/clang-format-9 /usr/bin/clang-format

FROM arm_base

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

WORKDIR /opt/src
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["bash"]
