#!/bin/bash
set -e

# If "-e uid={custom/local user id}" flag is not set for "docker run"
# command, use 9001 as default
DOCKER_UID=${LOCAL_USER_ID:-9001}
DOCKER_USERNAME="docker"
RUN_AS_USER="gosu ${DOCKER_USERNAME}"

# Notify user about the UID selected
echo "Welcome in Docker! Your user UID is: $DOCKER_UID"
# Create user called "docker" with selected UID
if id ${DOCKER_USERNAME} &>/dev/null; then
    echo "User already exists, skipping few steps"
else
    useradd --shell /bin/bash -u ${DOCKER_UID} -o -c "" -m ${DOCKER_USERNAME}

    # Force skel creation for mounted directories in home directory
    cp -r /etc/skel/. /home/${DOCKER_USERNAME}

    # Ensure the user has rights to home directory (can change if volume mount is forced)
    chown ${DOCKER_UID}:${DOCKER_UID} /home/${DOCKER_USERNAME}

    # Add to sudoers
    echo ${DOCKER_USERNAME} ALL=\(ALL\) NOPASSWD: ALL > /etc/sudoers.d/${DOCKER_USERNAME}
    chmod 0440 /etc/sudoers.d/${DOCKER_USERNAME}

    # Set "HOME" ENV variable for user's home directory
    export HOME=/home/${DOCKER_USERNAME}

    # Execute entrypoint dependencies of all layers as user
    if [ -d /docker/entrypoint.d ]; then
        for f in /docker/entrypoint.d/*; do
            ${RUN_AS_USER} bash $f
        done
    fi

    # --- VS Code remote development ---
    echo "Creating VS Code directories..."
    ${RUN_AS_USER} mkdir -p ~/.vscode-server{,-insiders}/extensions

    # Cache for XDG
    ${RUN_AS_USER} mkdir -p ~/.cache
    echo "export XDG_CACHE_HOME=/home/${DOCKER_USERNAME}/.cache" >> ~/.bashrc

    # Install runtime environment file
    DOCKERENV_FILE=/opt/src/.dockerenv
    if [ -f "${DOCKERENV_FILE}" ]; then
        ln -s ${DOCKERENV_FILE} /etc/profile.d/99-dockerenv.sh
    fi
fi

# Ensure the user has rights to home directory (once again both for existing
# user and new user, this time recursively).
chown -R ${DOCKER_UID}:${DOCKER_UID} /home/${DOCKER_USERNAME}

# Source modified environment. With positional args set, the Petalinux
# settings.sh script fails, hence as a workaround, clear them before sourcing.
SCRIPT_ARGS=("$@")
set --
source /etc/profile

# Execute GOSU process to drop shell to target user
exec gosu ${DOCKER_USERNAME} "${SCRIPT_ARGS[@]}"
