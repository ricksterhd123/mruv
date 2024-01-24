FROM ubuntu:22.04

ARG MRUBY_VERSION=3.2.0
ARG LIBUV_VERSION=1.47.0

RUN apt update && apt install -y build-essential
RUN apt update && apt install -y git
RUN apt update && apt install -y cmake
RUN apt update && apt install -y wget
RUN apt update && apt install -y curl
RUN apt update && apt install -y ruby

# grab mruby
RUN wget https://github.com/mruby/mruby/archive/refs/tags/${MRUBY_VERSION}.tar.gz -P /tmp/mruby
RUN tar -xf /tmp/mruby/${MRUBY_VERSION}.tar.gz -C /

# grab libuv
RUN wget https://github.com/libuv/libuv/archive/refs/tags/v${LIBUV_VERSION}.tar.gz -P /tmp/libuv
RUN tar -xf /tmp/libuv/v${LIBUV_VERSION}.tar.gz -C /

# install mruby
WORKDIR /mruby-${MRUBY_VERSION}
RUN rake
RUN rake install

# install libuv
WORKDIR /libuv-${LIBUV_VERSION}
RUN cmake . -DBUILD_TESTING=ON
RUN cmake --build .
RUN cmake --install .
