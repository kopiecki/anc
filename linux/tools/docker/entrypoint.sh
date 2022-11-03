#!/bin/bash
set -e

DOCKER_UID=${LOCAL_USER_ID:-9001}
DOCKER_USERNAME="docker"
 
if id ${DOCKER_USERNAME} &>/dev/null; then
    echo "Container is already configured"
else
    echo "Configuring container: creating non-root user..."

    useradd --shell /bin/bash -u $DOCKER_UID -o -c "" -m ${DOCKER_USERNAME}
    export HOME=/home/${DOCKER_USERNAME}
fi

# Execute process
echo "Docker ready, happy development!"
exec /usr/local/bin/gosu ${DOCKER_USERNAME} "$@"
