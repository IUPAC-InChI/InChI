FROM python:3.12 AS inchi_compilation


WORKDIR /inchi

# We need to preserve the git history in order to be able to check out different InChI versions.
COPY .git /inchi/.git
COPY .gitignore /inchi/.gitignore

# Include only what's necessary for compiling InChI and running the tests.
COPY INCHI-1-SRC/INCHI_BASE /inchi/INCHI-1-SRC/INCHI_BASE
COPY INCHI-1-SRC/INCHI_API /inchi/INCHI-1-SRC/INCHI_API
COPY INCHI-1-SRC/INCHI_EXE /inchi/INCHI-1-SRC/INCHI_EXE
COPY INCHI-1-TEST /inchi/INCHI-1-TEST
COPY CMakeLists.txt /inchi/CMakeLists.txt

# Ignore future changes, i.e., mounted custom files.
RUN echo "*" > /inchi/.gitignore

# We need to have a clean working tree in order to be able to check out different InChI versions.
# Since we a) copied only a subset of files from the original repo and b) modified .gitignore
# the working tree is dirty (i.e., there are uncommitted changes).
# That's why we're committing the changes now.
RUN git config user.name "inchi" && \
    git config user.email "" && \
    git add . && \
    git commit -m "Downsize repository"

RUN cd INCHI-1-TEST && ./install_build_dependencies.sh

# Compile current version (whatever commit is checked out).
RUN ./INCHI-1-TEST/build_with_cmake.sh all

# Compile previous non-minor version.
ENV exe_legacy_dir='/inchi/CMake_build/cli_build/bin/v1_6_0'
RUN mkdir -p $exe_legacy_dir
RUN ./INCHI-1-TEST/build_with_makefile.sh v1.06 $exe_legacy_dir exe || exit 1;

ENV lib_legacy_dir='/inchi/CMake_build/libinchi_build/lib/v1_6_0'
RUN mkdir -p $lib_legacy_dir
RUN ./INCHI-1-TEST/build_with_makefile.sh v1.06 $lib_legacy_dir lib || exit 1;

RUN cd INCHI-1-TEST && ./install_test_dependencies.sh

# Mount points for the `inchi-campaign` service's named volumes. Docker
# initialises a named volume from the image's content at that path and keeps its
# ownership, so creating them here -- owned by the UID the service runs as --
# is what makes them writable to a non-root container. Without it they arrive
# root-owned and the campaign cannot write a thing.
ARG CAMPAIGN_UID=1001
ARG CAMPAIGN_GID=1001
RUN mkdir -p /opt/campaign/venv /campaign/worktree && \
    chown -R ${CAMPAIGN_UID}:${CAMPAIGN_GID} /opt/campaign /campaign /inchi/CMake_build
