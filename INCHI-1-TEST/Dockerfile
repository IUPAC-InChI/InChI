FROM gcc:14-bookworm AS inchi_compilation
# The GCC image includes the dependencies we need for building InChI (gcc, make, etc.)
# and running the tests (Python 3.11).

WORKDIR /inchi

# We need to preserve the git history in order to be able to check out different InChI versions.
COPY .git /inchi/.git
COPY .gitignore /inchi/.gitignore

# Include only what's necessary for compiling InChI and running the tests.
COPY INCHI-1-SRC/INCHI_BASE /inchi/INCHI-1-SRC/INCHI_BASE
COPY INCHI-1-SRC/INCHI_API /inchi/INCHI-1-SRC/INCHI_API
COPY INCHI-1-SRC/INCHI_EXE /inchi/INCHI-1-SRC/INCHI_EXE
COPY INCHI-1-TEST /inchi/INCHI-1-TEST

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

ENV exe_dir='/inchi/CMake_build/cli_build'
ENV exe_legacy_dir="${exe_dir}/bin/v1_6_0"
ENV lib_dir='/inchi/CMake_build/libinchi_build'
ENV lib_legacy_dir="${lib_dir}/lib/v1_6_0"

# Compile current version (whatever commit is checked out).
RUN cmake -B $exe_dir -S 'INCHI-1-SRC/INCHI_EXE/inchi-1/src'
RUN cmake --build $exe_dir

RUN cmake -B $lib_dir -S 'INCHI-1-SRC/INCHI_API/libinchi/src'
RUN cmake --build $lib_dir

# Compile previous non-minor version.
RUN mkdir -p $exe_legacy_dir
RUN ./INCHI-1-TEST/build_with_makefile.sh v1.06 $exe_legacy_dir exe || exit 1;

RUN mkdir -p $lib_legacy_dir
RUN ./INCHI-1-TEST/build_with_makefile.sh v1.06 $lib_legacy_dir lib || exit 1;

FROM gcc:14-bookworm AS inchi_test

WORKDIR /inchi

# Include only what's necessary for running the tests.
COPY --from=inchi_compilation /inchi/CMake_build /inchi/CMake_build
COPY --from=inchi_compilation /inchi/INCHI-1-TEST/src /inchi/INCHI-1-TEST/src
COPY --from=inchi_compilation /inchi/INCHI-1-TEST/tests /inchi/INCHI-1-TEST/tests
COPY --from=inchi_compilation /inchi/INCHI-1-TEST/pyproject.toml /inchi/INCHI-1-TEST/pyproject.toml
COPY --from=inchi_compilation /inchi/INCHI-1-TEST/install_test_dependencies.sh /inchi/INCHI-1-TEST/install_test_dependencies.sh

RUN cd INCHI-1-TEST && ./install_test_dependencies.sh
