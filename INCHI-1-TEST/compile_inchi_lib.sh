#!/bin/bash

# generate libinchi.so.1 (symlink) and libinchi.so.dev

(cd INCHI-1-SRC/INCHI_API/libinchi/gcc &&
make -j LIB_DIR=../../../../INCHI-1-TEST VERSION=.dev &&
make clean)