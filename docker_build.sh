#!/usr/bin/bash

export DOCKER_BUILDKIT=1

chmod 777 /base/entrypoint
docker network create docker-shared
docker-compose build base
