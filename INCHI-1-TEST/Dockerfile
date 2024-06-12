FROM gcc:14-bookworm
# GCC image includes the dependencies we need for building InChI (gcc, make, etc.)
# and running the tests (Python 3.11).

WORKDIR /inchi_test
COPY pyproject.toml /inchi_test
COPY src/sdf_pipeline /inchi_test/src/sdf_pipeline
RUN apt update && apt install -y python3-pip
RUN python3 -m pip install --upgrade --break-system-packages pip && \
    python3 -m pip install --break-system-packages -e .[invariance-tests,development]
# Make `python3` available as `python`.
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10
WORKDIR /
