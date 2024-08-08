#!/bin/bash

# build library in inchi.dev directory
cmake -B inchi.dev && cmake --build inchi.dev

gcc_version=$(gcc --version | head -n 1)
inchi_version=$(git describe --tags)
echo "InChI $inchi_version compiled with $gcc_version."