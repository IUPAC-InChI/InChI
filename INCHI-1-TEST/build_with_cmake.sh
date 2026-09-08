#!/bin/bash

set -e

artifact=$1     # Must be "exe", "lib", "all".

if [ "$artifact" == "exe" ]; then
    source_dir="INCHI-1-SRC/INCHI_EXE/inchi-1/src"
    build_dir="CMake_build/cli_build"
elif [ "$artifact" == "lib" ]; then
    source_dir="INCHI-1-SRC/INCHI_API/libinchi/src"
    build_dir="CMake_build/libinchi_build"
elif [ "$artifact" == "all" ]; then
    source_dir="."
    build_dir="CMake_build/full_build"
else
    echo "Invalid artifact type: $artifact. Must be 'exe', 'lib', or 'all'."
    exit 1
fi

cmake -B $build_dir -S $source_dir -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build $build_dir
