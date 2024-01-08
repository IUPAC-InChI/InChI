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


This directory contains inchi-1 source code and executables
(command-line executables produce standard and non-standard InChI/InChIKey).


=========
  FILES 
=========

readme.txt        This file

common            SUB-DIRECTORY
                  Part of the source code common with InChI library

main		  SUB-DIRECTORY
                  The rest of the source code

gcc               SUB-DIRECTORY
    inchi-1           SUB-DIRECTORY
                      Contains gcc makefile to create inchi-1 executable

vc9               SUB-DIRECTORY
    inchi-1           SUB-DIRECTORY
                      Contains Microsoft Visual C++ 2009 project 
                      to create inchi-1.exe
    cmlpp             SUB-DIRECTORY
                      Contains Microsoft Visual C++ 2009 project 
                      to add CML support to inchi-1.exe
                      

The inchi-1.exe (Win32 executable) with CML input support created
with Microsoft Visual C++ 2009 is in INCHI-1-BIN archive file of
this distribution.
