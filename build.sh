#!/usr/bin/env
set -e

VERSION="latest"
docker build . --build-arg DOCKER_USER=$(whoami) -t mruv:$VERSION
docker container run -v ".:/mruv" mruv:$VERSION 'cd mruv && mkdir -p build && cd build && cmake .. && cmake --build .'
