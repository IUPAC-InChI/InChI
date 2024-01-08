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
This is the release of the IUPAC International Chemical Identifier 
with InChIKey, version 1, software version 1.03.
(http://www.iupac.org/projects/2000/2000-025-1-800.html
http://www.iupac.org/inchi).

This software supports both standard and non-standard InChI/InChIKey.
*****


This directory contains a number of sample files for testing InChI/InChIKey
programs. In particular, included are the files containing (most of) chemical 
structures drawn in InChI User's Guide and Technical Manual.


=========
  FILES 
=========

readme.txt                       This file

UserGuide                        SUB-DIRECTORY
    (S)-Glutamic_Acid.mol        3 sample files -- illustrations from the
    benzoicacid.mol              InChI User's Guide
    Benzoic_Acid_Sodium_salt.mol

TechMan                          SUB-DIRECTORY
    SamplesTechMan.sdf           Examples of structures from the
                                 Technical Manual
    SamplesTechMan.zip           Individual structures from the
                                 Technical Manual

Misc                             SUB-DIRECTORY
    Samples.sdf                  Sample structure files merged into one
                                 SDfile
    Samples.zip                  Individual structure sample files

CML                              SUB-DIRECTORY
    SL0010_three_dimensions.cml  3D structure in CML format
    SL0010_two_dimensions.cml    Same 2D structure in CML format
    SL0010_zero_dimensions.cml   Same 0D structure in CML format
                                 (on the display all atoms overlap
                                 due to all coordinates being equal
                                 to zero)
    SL0010_three_dimensions.mol  Same 3D structure in Molfile format

    M0005C.cml                   A collection of structures in CML format


Note.
------
When using winchi-1 exe, you may enter word

NAME

into "Structure ID Header" of an "Open" dialog 
to view structure name in Samples and SamplesTechMan collections.
