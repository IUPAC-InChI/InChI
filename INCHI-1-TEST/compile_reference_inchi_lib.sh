#!/bin/bash

reference_version="v1.06"

if ! git diff-index --quiet HEAD --; then
    echo "There are uncommitted changes on the current branch."
    echo "Please commit or stash them before checking out the reference tag."
    exit 1
fi
git checkout tags/$reference_version

(cd INCHI-1-SRC/INCHI_API/libinchi/gcc &&
make -j LIB_DIR=../../../../INCHI-1-TEST VERSION=.$reference_version)

gcc_version=$(gcc --version | head -n 1)
inchi_version=$(git describe --tags)

# Switch back to the branch that was checked out prior to checking out the tag.
git switch -

echo "InChI $inchi_version compiled with $gcc_version."
