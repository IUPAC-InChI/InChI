#!/bin/bash

wget https://raw.githubusercontent.com/IUPAC-InChI/InChI_1_06/HEAD/INCHI-1-BIN/linux/64bit/so/libinchi.so.1.06.00.gz
gunzip libinchi.so.1.06.00.gz
mv libinchi.so.1.06.00 INCHI-1-TEST/libinchi.so.1.06.00