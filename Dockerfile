FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
SHELL ["/bin/bash", "-l", "-c"]

ARG MRUBY_VERSION=3.2.0
ARG LIBUV_VERSION=1.47.0
ARG RUBY_VERSION=3.2.3

RUN apt update && apt install -y build-essential
RUN apt update && apt install -y git
RUN apt update && apt install -y cmake
RUN apt update && apt install -y gdb 
RUN apt update && apt install -y wget
RUN apt update && apt install -y curl
RUN apt update && apt install -y gnupg2
RUN apt update && apt install -y netcat
RUN apt update && apt install -y libpthread-stubs0-dev

# grab rvm && install ruby
RUN gpg2 --keyserver keyserver.ubuntu.com --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 7D2BAF1CF37B13E2069D6956105BD0E739499BDB
RUN curl -sSL https://get.rvm.io | bash -s stable
RUN rvm install ${RUBY_VERSION}
RUN rvm --default use ${RUBY_VERSION}

# install ruby gems for development environment
RUN gem install ruby-lsp
RUN gem install solargraph
RUN gem install rubocop

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

# setup as a devcontainer and buildtool
WORKDIR /
ENTRYPOINT ["/bin/bash", "-l", "-c"]
