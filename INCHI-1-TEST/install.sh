#!/bin/bash

apt update && apt install -y python3-pip cmake
python3 -m pip install --upgrade --break-system-packages pip
python3 -m pip install --break-system-packages -e .[invariance-tests]
# Make `python3` available as `python`.
update-alternatives --install /usr/bin/python python /usr/bin/python3 10
