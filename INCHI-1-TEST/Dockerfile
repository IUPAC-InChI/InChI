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

# v1.03, v1.04 don't compile.
ENV inchi_versions="v1.05 v1.06 v1.07.0 main"
ENV lib_dir="/inchi/INCHI-1-TEST/libs"
RUN mkdir $lib_dir && for version in $inchi_versions; do \
    /inchi/INCHI-1-TEST/compile_inchi_lib.sh "$version" "$lib_dir" || exit 1; \
    done

FROM gcc:14-bookworm AS inchi_test

WORKDIR /inchi

# Include only what's necessary for running the tests.
COPY --from=inchi_compilation /inchi/INCHI-1-TEST/src /inchi/INCHI-1-TEST/src
COPY --from=inchi_compilation /inchi/INCHI-1-TEST/libs /inchi/INCHI-1-TEST/libs
COPY --from=inchi_compilation /inchi/INCHI-1-TEST/pyproject.toml /inchi/INCHI-1-TEST/pyproject.toml
COPY --from=inchi_compilation /inchi/INCHI-1-TEST/install.sh /inchi/INCHI-1-TEST/install.sh

# Install test environment.
RUN cd INCHI-1-TEST && ./install.sh
