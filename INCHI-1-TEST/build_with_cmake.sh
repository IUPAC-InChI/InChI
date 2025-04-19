#!/bin/bash

set -e

artifact=$1     # Must be "exe" or "lib".

if [ "$artifact" == "exe" ]; then
    source_dir="INCHI-1-SRC/INCHI_EXE/inchi-1/src"
    build_dir="CMake_build/cli_build"
elif [ "$artifact" == "lib" ]; then
    source_dir="INCHI-1-SRC/INCHI_API/libinchi/src"
    build_dir="CMake_build/libinchi_build"
else
    echo "Invalid artifact type: $artifact. Must be 'exe' or 'lib'."
    exit 1
fi

cmake -B $build_dir -S $source_dir
cmake --build $build_dir
