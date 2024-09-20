#!/bin/bash

set -e

inchi_version=$1    # Must be one of the tags in `git tag`, branches in `git branch`, or a commit hash.
inchi_dir=$2        # Must be absolute.
artifact=$3         # Must be "exe" or "lib".

if [ "$artifact" == "exe" ]; then
    makefile_dir="INCHI-1-SRC/INCHI_EXE/inchi-1/gcc"
    make_args="BIN_DIR=$inchi_dir"
elif [ "$artifact" == "lib" ]; then
    makefile_dir="INCHI-1-SRC/INCHI_API/libinchi/gcc"
    make_args="LIB_DIR=$inchi_dir VERSION=.$inchi_version"
else
    echo "Invalid artifact type: $artifact. Must be 'exe' or 'lib'."
    exit 1
fi

current_branch=$(git rev-parse --abbrev-ref HEAD)
checkout_successful=0
cleanup() {
    # Starting with v1.07.0 there's a clean target in the makefile (`make -C $makefile_dir clean`).
    # However, for consistency across versions we're manually removing the object files
    # that are left over from compilation.
    find $makefile_dir -name "*.o" -type f -delete
    # Switch back to the branch that was checked out prior to checking out <inchi_version>.
    git checkout "$current_branch"
}
# Register cleanup to be called on EXIT.
trap cleanup EXIT

if [ -n "$(git status --porcelain)" ]; then
    echo "There are uncommitted changes on the current branch."
    echo "Please commit or stash them before compiling '$inchi_version'."
    exit 1
fi

if git rev-parse --verify --quiet refs/tags/"$inchi_version"; then
    echo "Checking out version tag '$inchi_version'."
    git checkout "tags/$inchi_version"
    checkout_successful=1
fi

if git rev-parse --verify --quiet "$inchi_version"; then
    echo "Checking out branch or commit '$inchi_version'."
    git checkout "$inchi_version"
    checkout_successful=1
fi

if [ $checkout_successful -eq 0 ]; then
    echo "'$inchi_version' does not exist."
    exit 1
fi

echo "$make_args" | xargs make -C $makefile_dir -j
