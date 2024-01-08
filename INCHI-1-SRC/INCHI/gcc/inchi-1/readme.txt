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



This directory contains gcc makefiles to create inchi-1 executable.

The makefiles were tested with
- gcc version 3.4.5 (mingw-special) under WinXP Pro SP2;
- gcc v. 4.1.2 20061115 (Debian 4.1.1-21) under Debian Etch;
- gcc v. 4.2.4 under Ubuntu 2.6.24-24-server.

The makefile should be placed into a directory that has a common parent with 
"main" and "common" directories as it is in the zip file containing the source 
code and this makefile.

To use function times() instead of clock() for detecting timeout remove -ansi 
compiler option and add option -DINCHI_USETIMES


=========
  FILES 
=========

readme.txt        This file

makefile          Creates:
                  32-bit executable under 32-bit Windows and 32-bit Linux,
                  or 64-bit executable under 64-bit Linux
    
makefile32        Use to make 32-bit executable under 64-bit Linux



=========
  LINKS
=========

IUPAC                   http://www.iupac.org/inchi
InChI Trust             http://www.inchi-trust.org                                      
InChI discussion group  https://lists.sourceforge.net/lists/listinfo/inchi-discuss
