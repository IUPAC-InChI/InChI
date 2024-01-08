/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.05
 * January 27, 2017
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST. 
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file 
 * 'External-contributors' included in this distribution.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the 
 * International Chemical Identifier (InChI) 
 * Copyright (C) IUPAC and InChI Trust Limited
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0, 
 * or any later version.
 * 
 * Please note that this library is distributed WITHOUT ANY WARRANTIES 
 * whatsoever, whether expressed or implied.  
 * See the IUPAC/InChI-Trust InChI Licence No.1.0 for more details.
 * 
 * You should have received a copy of the IUPAC/InChI Trust InChI 
 * Licence No. 1.0 with this library; if not, please write to:
 * 
 * The InChI Trust
 * 8 Cavendish Avenue
 * Cambridge CB1 7US
 * UK
 *
 * or e-mail to alan@inchi-trust.org
 * 
 */


This directory contains binaries of command line InChI executable (inchi-1) 
and InChI API library (libinchi). The 32 and 64 bit versions are supplied 
for both Windows and Linux. 

Also included is winchi-1.exe, a graphical Windows application
(directory 'windows'; a 32 bit version which will also run under 64 bit Windows).

Note that:

* InChI stand-alone executable inchi-1[.exe] does not require InChI dll/so libraries.

* To use the shared library, you may wish to create 'libinchi.so.1' as a symbolic link 
  to 'libinchi.so.1.05.00'
