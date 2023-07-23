#!/usr/bin/bash

docker-compose run -e LOCAL_USER_ID=$(id -u) base /bin/bash
