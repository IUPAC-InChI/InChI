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


This directory contains a simple test suite for InChI version 1, 
software version 1.06.

Included are: 
a) zipped source structural data (in SDF format);
b) zipped reference result files containing InChI strings (in several cases, 
accompanied by InChIKey strings) generated from the source data by InChI 
software v. 1.06 executable inchi-1 with different command-line options;
c) scripts used to generate the reference identifiers.

The source data includes a minimal set of chemical structures presented 
as a single SDfile, InChI_TestSet.sdf. Samples containing polymer
structures and Zz pseudo atoms are in zzp.sdf dataset.
