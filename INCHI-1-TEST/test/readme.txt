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


This directory contains a simple test suite for InChI version 1, 
software version 1.03.

Included are: 
a) source structural data (in SDF format);
b) 36 reference result files containing InChI strings (in several cases, 
accompanied by InChIKey strings) generated from the source data by InChI 
software v. 1.03 executable inchi-1 with different command-line options;
c) scripts used to generate the reference identifiers.

Using these data, one may verify that InChI code included in an 
application or InChI code ported to a particular compiler or operating 
system (such an application containing InChI code later in this document 
is called "software") produces the same InChI as the software distributed by 
IUPAC.

The method of verification is to compare InChI produced by the software out 
of a representative set of chemical structures and with various options 
to InChI produced by the official IUPAC software. 
Identical results are necessary for the software to pass the verification.
The same procedure should be used for INChIKey, when appicable.

The source data includes a minimal set of chemical structures presented 
as a single SDfile, InChI_TestSet.sdf.

The 6 sets of options were used for generation of reference standard 
identifiers and the 30 sets of options were used to produce non-standard
ones (note that this does not provide the exhaustive coverage of all possible
option combinations).

The options and corresponding reference output file names are given below.


File                          InChIKey  InChI options

* standard InChI *

InChI_TestSet-std-01.txt      yes       -
InChI_TestSet-std-02.txt      no        NEWPSOFF
InChI_TestSet-std-03.txt      yes       SNon
InChI_TestSet-std-04.txt      no        DoNotAddH
InChI_TestSet-std-05.txt      no        SNon DoNotAddH
InChI_TestSet-std-06.txt      no        NEWPSOFF DoNotAddH

* non-standard InChI *

InChI_TestSet-non-std-01.txt  no        SUU SLUUD
InChI_TestSet-non-std-02.txt  yes       SRel 
InChI_TestSet-non-std-03.txt  no        SRac
InChI_TestSet-non-std-04.txt  no        SUU SLUUD SUCF
InChI_TestSet-non-std-05.txt  no        NEWPSOFF SRel
InChI_TestSet-non-std-06.txt  no        NEWPSOFF SRac
InChI_TestSet-non-std-07.txt  no        NEWPSOFF SLUUD SUCF
InChI_TestSet-non-std-08.txt  yes       FixedH 
InChI_TestSet-non-std-09.txt  no        NEWPSOFF FixedH 
InChI_TestSet-non-std-10.txt  no        FixedH SNon 
InChI_TestSet-non-std-11.txt  no        FixedH SRel 
InChI_TestSet-non-std-12.txt  yes       RecMet 
InChI_TestSet-non-std-13.txt  no        NEWPSOFF RecMet 
InChI_TestSet-non-std-14.txt  no        RecMet SNon 
InChI_TestSet-non-std-15.txt  no        RecMet SRel 
InChI_TestSet-non-std-16.txt  yes       FixedH RecMet 
InChI_TestSet-non-std-17.txt  no        NEWPSOFF FixedH RecMet 
InChI_TestSet-non-std-18.txt  no        FixedH RecMet SNon 
InChI_TestSet-non-std-19.txt  no        FixedH RecMet SRel 
InChI_TestSet-non-std-20.txt  yes       KET 
InChI_TestSet-non-std-21.txt  no        KET SNon
InChI_TestSet-non-std-22.txt  no        KET SRel
InChI_TestSet-non-std-23.txt  yes       15T 
InChI_TestSet-non-std-24.txt  no        15T SNon
InChI_TestSet-non-std-25.txt  no        15T SRel
InChI_TestSet-non-std-26.txt  no        KET 15T
InChI_TestSet-non-std-27.txt  yes       NEWPSOFF KET 15T
InChI_TestSet-non-std-28.txt  no        NEWPSOFF FixedH KET 15T Key 
InChI_TestSet-non-std-29.txt  no        NEWPSOFF SUU SLUUD KET 15T 
InChI_TestSet-non-std-30.txt  no        NEWPSOFF SUU SLUUD RecMet KET 15T 


To reduce the size of the output, two additional options are added to each set:
/AuxNone /NoLabels.
If InChIKey should be printed, the option
/Key
is specified.

For example, the 01 set of std InChI options is:
/AuxNone /NoLabels /Key


Note. In general, this test suite establishes necessary but not sufficient 
conditions for the software compliance: it does not prove that there does 
not exist a structure such that the identifier produced by the software 
would be different from that produced by inchi-1. 
A practical solution is to test the software on as great as possible 
variety of structures.  Therefore, at the discretion of the tester, 
other structure collection(s) may be included in validation. 

Such a collection is, for example, NCI Open "September 2003 SD File of 
Combined DTP Releases, 2D/3D, with Canonical Properties Added," containing 
260,071 structures. The compressed SDfile may be downloaded from 
http://cactus.nci.nih.gov/download/nci/
The direct link to the file is "260,071 structures in SDF format",
http://cactus.nci.nih.gov/download/nci/NCI-Open_09-03.sdf.gz

Another publicly available collections is the PubChem collection of 
Compound chemical structures. It may be found at
ftp://ftp.ncbi.nlm.nih.gov/pubchem/Compound/CURRENT-Full/SDF/

 
=========
  FILES 
=========

readme.txt                       This file

InChI_TestSet.sdf                Source structural data (SDF)

TestSet2InChI.bat                Windows batch file for generation of identifiers
TestSet2InChI.sh                 Linux shell script for generation of identifiers

InChI_TestSet-result.zip         Archive containing the 36 reference 
                                 result files:
                                     (standard)
                                     InChI_TestSet-std-01.txt
                                     ...
                                     InChI_TestSet-std-06.txt
                                     (non-standard)
                                     InChI_TestSet-non-std-01.txt
                                     ...
                                     InChI_TestSet-non-std-30.txt
