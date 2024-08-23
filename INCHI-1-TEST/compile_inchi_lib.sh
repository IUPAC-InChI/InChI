#!/bin/bash

set -e

inchi_version=$1 # Must be one of the tags in `git tag` or branches in `git branch`.
inchi_lib_dir=$2 # Path must be absolute.
current_branch=$(git rev-parse --abbrev-ref HEAD)
makefile_dir="INCHI-1-SRC/INCHI_API/libinchi/gcc"
checkout_successful=0
cleanup() {
    # Starting with v1.07.0 there's a clean target in the makefile (`make -C $makefile_dir clean`).
    # However, for consistency across versions we're manually removing the object files
    # that are left over from compilation.
    find $makefile_dir -name "*.o" -type f -delete
    # Switch back to the branch that was checked out prior to checking out <inchi_version>.
    git checkout $current_branch
}
# Register cleanup to be called on EXIT.
trap cleanup EXIT

if [ -n "$(git status --porcelain)" ]; then
    echo "There are uncommitted changes on the current branch."
    echo "Please commit or stash them before compiling '$inchi_version'."
    exit 1
fi

# Check out the specified tag or branch.
if git rev-parse --verify --quiet refs/tags/$inchi_version; then
    echo "Checking out version tag '$inchi_version'."
    git checkout "tags/$inchi_version"
    checkout_successful=1
fi

if git rev-parse --verify --quiet $inchi_version; then
    echo "Checking out branch '$inchi_version'."
    git checkout $inchi_version
    checkout_successful=1
fi

if [ $checkout_successful -eq 0 ]; then
    echo "Version tag or branch '$inchi_version' does not exist."
    exit 1
fi

# Generate libinchi.so.<inchi_version>.
make -C $makefile_dir -j LIB_DIR=$inchi_lib_dir VERSION=.$inchi_version
