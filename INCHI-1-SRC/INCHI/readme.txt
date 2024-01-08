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



This directory contains inchi-1 source code and executables
(command-line executables produce standard and non-standard InChI/InChIKey).


=========
  FILES 
=========

readme.txt        This file

common            SUB-DIRECTORY
                  Part of the source code common with InChI library

main		      SUB-DIRECTORY
                  The rest of the source code

gcc               SUB-DIRECTORY
    inchi-1       SUB-DIRECTORY
                      Contains gcc makefile to create inchi-1 executable

vc9               SUB-DIRECTORY
    inchi-1       SUB-DIRECTORY
                      Contains Microsoft Visual Studio 2008 project 
                      to create inchi-1.exe
                      

The inchi-1.exe created with Microsoft Visual Studio 2008 is in INCHI-1-BIN 
section of this distribution.



=========
  LINKS
=========

IUPAC                   http://www.iupac.org/inchi
InChI Trust             http://www.inchi-trust.org                                      
InChI discussion group  https://lists.sourceforge.net/lists/listinfo/inchi-discuss
