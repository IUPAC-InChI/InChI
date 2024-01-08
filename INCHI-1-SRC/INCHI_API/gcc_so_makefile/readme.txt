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



This directory contains  a gcc makefile for INCHI_MAIN + INCHI_DLL code 
to create a InChI library as a shared object, as well as the demo executable
calling the library (Linux).

This makefile was tested with gcc v. 4.1.2 20061115 (Debian 4.1.1-21) 
under Debian Etch and gcc v. 4.2.4 under Ubuntu 2.6.24-24-server.


How to run make under Linux
---------------------------
make ISLINUX=1

This would restrict a list of exported entry points to those described
in inchi_api.h (see file inchi.map), add CWD to the inchi_main shared
object search path, and, most importantly, eliminate refusal to load
the shared library which otherwise would produce this message:

"error while loading shared libraries: libinchi.so.1:
 cannot restore segment prot after reloc: Permission denied"

under SELinux default settings.

Note: it may be necessary to copy symlink libinchi.so.1 to /usr/lib.


General
-------
This makefile creates InChI library and a dynamically linked
to it demo application, inchi_main. 

Note that the inchi_main is just a sample which is not supposed 
to be used for the production.

The binaries consist of:

1) The main program -- a Molfile/InChI Aux Info reader
that creates a chemical structure representation suitable for
the InChI library API, feeds it into the InChI library and
outputs the results.
The code is located in the ../inchi_main sub-directory.

2) The InChI library that creates the standard InChI identifier,
standard InChIKey, Auxiliary information, and error/warning messages.
The code is located in the ../inchi_dll sub-directory

The makefile should be placed into a directory that has a common parent 
with INCHI_MAIN and INCHI_DLL as it is in the zip file containing
the source code and this makefile.

Under Linux the makefile creates:
 inchi_main              -- the main program (InChI software library 
                            demo application) and
 libinchi.so.1.04.00     -- the shared object and a link libinchi.so.1 to it
 libinchi.so.1           -- a symbolic link to libinchi.so.1.04.00

in the "result" subdirectory of this makefile directory out of source
code located in "INCHI_DLL" and "INCHI_MAIN" directories.

Files result/libinchi.so.1.04.00.gz and result/inchi_main.gz
contain Linux i386 binaries created with the included makefile.

The InChI software library demo application, inchi_main, needs
libinchi.so.1, a symbolic link to libinchi.so.1.04.00.



=========
  LINKS
=========

IUPAC                   http://www.iupac.org/inchi
InChI Trust             http://www.inchi-trust.org                                      
InChI discussion group  https://lists.sourceforge.net/lists/listinfo/inchi-discuss
