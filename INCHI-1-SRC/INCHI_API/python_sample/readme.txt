/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.04
 * September 9, 2011
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST. Modifications and additions by IUPAC 
 * and the InChI Trust.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the 
 * International Chemical Identifier (InChI) Software version 1.04
 * Copyright (C) IUPAC and InChI Trust Limited
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0, 
 * or any later version.
 * 
 * Please note that this library is distributed WITHOUT ANY WARRANTIES 
 * whatsoever, whether expressed or implied.  See the IUPAC/InChI Trust 
 * Licence for the International Chemical Identifier (InChI) Software 
 * version 1.04, October 2011 ("IUPAC/InChI-Trust InChI Licence No.1.0") 
 * for more details.
 * 
 * You should have received a copy of the IUPAC/InChI Trust InChI 
 * Licence No. 1.0 with this library; if not, please write to:
 * 
 * The InChI Trust
 * c/o FIZ CHEMIE Berlin
 *
 * Franklinstrasse 11
 * 10587 Berlin
 * GERMANY
 *
 * or email to: ulrich@inchi-trust.org.
 * 
 */


This directory contains Python demo application illustrating how the InChI 
library functions may be called from within Python. It has a simple Mol/SDfile 
reader and produces InChI strings for a SD file and, optionally, generates 
InChIKey.

Tested with Python 2.7.2 64 bit under Windows 7 64bit and 
Python 2.5.2 under Ubuntu 10.04.3 LTS.

It is assumed that under MS Windows the InChI library name is 'libinchi.dll' 
while under Linux it is '/usr/lib/libinchi.so.1' (see readme.txt file in 
INCHI_API/gcc_so_makefile sub-directory for the description of InChI software 
library creation).

Note that this Python program should not be used as an InChI generator. The 
code in this example is provided for illustrative purposes only. It does not 
extensively check the input data and does not provide any diagnostic concerning 
input structure(s). Due to its simplicity, this SDfile reader cannot interpret
correctly some of molfile features properly interpreted by inchi-1.		


Usage
-----
make_inchi.py [options]


Options:
  -h, --help            show this help message and exit
  -i INPUT, --input=INPUT
                        name of input SD file (required)
  -o OUTPUT, --output=OUTPUT
                        name of output file (default=inchi_out.txt)
  -l LOG, --log=LOG     name of log file (errors/warnings; default=stdout)
  -s START_RECORD, --start_record=START_RECORD
                        starting number of record to be converted
  -e END_RECORD, --end_record=END_RECORD
                        number of the last record to be converted
  -x, --aux             print auxilary info
  -k, --key             calculate InChIKey
  -z, --xtra            calculate extra hash complementing InChIKey
  -p INCHI_OPTIONS, --inchi_options=INCHI_OPTIONS
                        string with InChI options


   
=========
  FILES 
=========

readme.txt        This file

make_inchi.py     Python demo app, main program

PyINCHI.py        Python interface to INCHI library

lightsdf.py	      Light SDF reader: parses SD file records and collects 
                  information



=========
  LINKS
=========

IUPAC                   http://www.iupac.org/inchi
InChI Trust             http://www.inchi-trust.org                                      
InChI discussion group  https://lists.sourceforge.net/lists/listinfo/inchi-discuss
