FROM python:3.12-bookworm
# Python image includes the dependencies we need for building InChI (gcc, make, etc.).

WORKDIR /inchi_test
COPY . /inchi_test

RUN python -m pip install --upgrade pip && \
    python -m pip install -e .[invariance-tests,development]

# On Debian bookworm (12) only GCC versions 11 and 12 are available.
# For GCC versions 9 and 10, use Debian bullseye (11).
ARG GCC_VERSION=12
RUN apt-get update && apt-get install gcc-$GCC_VERSION -y
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$GCC_VERSION 42
RUN update-alternatives --set gcc /usr/bin/gcc-$GCC_VERSION
