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



This directory includes 'command line' InChI executable and InChI API library binaries.
The 32 and 64 bit versions are supplied for both Windows and Linux. 

Also included is winchi-1.exe, a graphical Windows application
(directory 'windows'; a 32 bit version which will also run under 64 bit Windows).


=========
  FILES 
=========

readme.txt                          This file

WINDOWS                             SUB-DIRECTORY
    winchi-1.exe                    InChI graphical Windows application 
    
    32BIT                           SUB-DIRECTORY
        inchi-1.exe                 InChI stand-alone command line executable, 32 bit

        DLL                         SUB-DIRECTORY
            libinchi.dll            InChI dynamic-link library, 32 bit

    64BIT                           SUB-DIRECTORY
        inchi-1.exe                 InChI stand-alone command line executable, 64 bit

        DLL                         SUB-DIRECTORY
            libinchi.dll            InChI dynamic-link library, 64 bit

LINUX                               SUB-DIRECTORY

    32BIT                           SUB-DIRECTORY
        inchi-1.gz                  InChI stand-alone command line executable, 32 bit; gzipped
        
        SO                          SUB-DIRECTORY
            libinchi.so.1.04.00.gz  shared library for InChI API, 32 bit; gzipped
            
    64BIT                           SUB-DIRECTORY
        inchi-1.gz                  InChI stand-alone command line executable, 64 bit; gzipped
        
        SO                          SUB-DIRECTORY
            libinchi.so.1.04.00.gz  shared library for InChI API, 64 bit; gzipped



Note-1. InChI stand-alone executable inchi-1[.exe] does not require dll/so libraries.

Note-2. Example programs which use InChI API library for both Windows (dll) and Linux (so)
are supplied in INCHI-1-API/InChI_API section of this distribution package.
There are examples for C ('inchi_main' application, see projects for MS Visual Studio 2008 'vc9' 
and gcc 'gcc_so_makefile' ) and Python ('python_sample'). Also supplied ther
are InChI API library sorce codes and related MS Visual Studio 2008/gcc projects.

Note-3. To use the shared library, you may wish to create 'libinchi.so.1' 
as a symbolic link to 'libinchi.so.1.04.00'



=========
  LINKS
=========

IUPAC                   http://www.iupac.org/inchi
InChI Trust             http://www.inchi-trust.org                                      
InChI discussion group  https://lists.sourceforge.net/lists/listinfo/inchi-discuss
