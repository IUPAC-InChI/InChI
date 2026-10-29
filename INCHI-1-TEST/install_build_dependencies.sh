#!/bin/bash

apt update
apt install -y cmake doxygen graphviz gdb lcov locales

# Locales for the GH #100 tests: comma and multi-byte (U+066B) decimal points
localedef -i de_DE -f UTF-8 de_DE.UTF-8
localedef -i ps_AF -f UTF-8 ps_AF.UTF-8
