/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.03
 * June 15, 2010
 *
 * Originally developed at NIST
 * Modifications and additions by IUPAC and the InChI Trust
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC);
 * you can redistribute this software and/or modify it under the terms of 
 * the GNU Lesser General Public License as published by the Free Software 
 * Foundation:
 * http://www.opensource.org/licenses/lgpl-2.1.php
 */

*****
This is the release of the IUPAC International Chemical
Identifier with InChIKey, version 1, software version 1.03.
(http://www.iupac.org/projects/2000/2000-025-1-800.html
http://www.iupac.org/inchi).

This software supports both standard and non-standard InChI/InChIKey.
*****


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
