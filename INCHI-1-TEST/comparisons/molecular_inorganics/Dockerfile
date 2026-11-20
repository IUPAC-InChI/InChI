# Toolchain for run_comparison.sh. Nothing from the repository is copied in:
# the service bind-mounts the working tree, and the script builds both libraries
# and its virtualenv itself, onto the named volumes in docker-compose.yml.
#
# python:3.12 already has git, gcc, make and ensurepip; cmake is the only gap.
FROM python:3.12

RUN apt-get update && \
    apt-get install -y --no-install-recommends cmake && \
    rm -rf /var/lib/apt/lists/*

# Mount points for the named volumes. Docker initialises a named volume from the
# image's content at that path and keeps its ownership, so creating them here --
# owned by the UID the service runs as -- is what makes them writable to a
# non-root container. Without it they arrive root-owned and nothing can be built.
ARG COMPARISON_UID=1001
ARG COMPARISON_GID=1001
RUN mkdir -p /opt/comparison/venv /comparison/worktree /inchi/CMake_build && \
    chown -R ${COMPARISON_UID}:${COMPARISON_GID} \
        /opt/comparison /comparison /inchi/CMake_build

WORKDIR /inchi
