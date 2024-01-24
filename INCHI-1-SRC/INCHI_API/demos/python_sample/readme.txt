/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.06
 * December 15, 2020
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
 * Copyright (C) IUPAC and InChI Trust
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
 * Licence No. 1.0 with this library; if not, please e-mail:
 *
 * info@inchi-trust.org
 *
 */


This directory contains Python demo application illustrating how the InChI 
Software Library functions (API) may be called from within Python. 
It reads SD/MOL input file and produces InChI strings and, optionally, 
generates InChIKey's.

The program uses Python 3; tested with Python 3.7.1 64 bit under Windows 10 64-bit 
and Python 3.4.5 under Linux CentOS 7.7.

It is assumed that under MS Windows the InChI library name is 'libinchi.dll' 
while under Linux it is '/usr/lib/libinchi.so.1'.

Note that this Python program is not indended to be a production InChI generator. 
The example is provided for illustrative purposes only. 

For usage instructions, type "mol2inchi.py" without parameters.
