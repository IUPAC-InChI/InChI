# Changelog

## v1.04 release 2011-10-12

### Added

- Support for chemical element numbers 105-112 (see release notes for detailed information)
- Possibilty of processing mult iple input files at a single run added to inchi-executable (both on Windows and Linux versions); common file name wildcards are recognized

### Changed

- Software licence changed from GNU Lesser General Public Licence (LGPL) version 2.1 to the more permissive IUPAC/InChI-Trust Licence for International Chemical Identifier (InChI) Software version 1.04, September 2011 ("IPUCA/InChI-Trust Licence No. 1.0")
- Standard valences for No, Lr, and Rf were updated
- Several minor changes in source code

### Removed

- Removed support for CML input files and related source code
- Removed unnecessary dependence of the inchi-1 executable for Windows on Microsoft run-time dynamic libraries was eliminated

### Fixed

- Fixed bug in the normalization procedure for some structures (containing a radical at an atom in an aromatic ring) which may result in different InChI strings for the same molecule, depending on the original order of the atomic numbers; this fix repairs the issue reported Dmitry Pavlov, SciTouch

## v1.03 release 2010-06-15

### Added

- New command line option `SLUUD` to distinguish stereo labels 'u' for "unknown" and '?' for "undefined"
- Experimental new line options `KET` and `15T` provide access to advanced tautomerism detection; `/Ket` (`-Ket` under Linux) for keto-enol tautomerism; `/15T` (`-15T` under Linux) for 1,5-tautomerism
- New structure perception option `NEWPSOFF` points both ends of wedge bonds to stereocenters
- New structure perception option `DoNotAddH` makes all hydrogens in input structure explicit
- New structure perception option `SNon` to ignore stereo
- New structure perception option `SRel`/`SRac` to use relative/racemic stereo
- New structure perception option `SUCF` to use chiral flag in MOL/SD file record: if On - use absolute stereo, Off - relative
- New InChI creation option `SUU` to always indicate unknown/undefined stereo
- New InChI creation option `RecMet` to include reconnected metals in results
- New InChI creation option `FixedH` to include "Fixed H layer"
- New command line option `SaveOpt` allows to append non-standard InChI string wih saved InChI creation options
- New command line option `XHash1` allows to output the res of 256-bit SHA-2 signature for 1<sup>st</sup> block
- New command line option `XHash2` allows to output the res of 256-bit SHA-2 signature for 2<sup>nd</sup> block

### Changed

- For `InChI2InChI` conversion generating standard InChI strings out of standard ones is disabled; conversion to non-standard ones is allowed (see release notes for detailed information)
- Minor technical clarification is made in references to GNU Lesser General Public License, LGPL
- Source code of CML reade is published with InChI package, as free software under the same LGPL licence as InChI itself, thanks to Peter Murray-Rust and Simon (Billy) Tyrell, Unilever Centre for Molecular Sciences Informatics, University of Cambridge, UK

### Fixed

- Fixed bug in normalization procedure for some structures (containing N2(+) fragment) which may result in producing different InChI strings for the same molecule, depending on original order of the atomic numbers - thanks to Timo Boehme
- Fixed bug in normalization for structures containing positively charged tetra-coordinated phosphorus (or sulfur) connected to the negatively charged oxygen - thanks to Hinnerk Rey
- Fixed bug in treating stereochemistry of allenes with non-carbon substituents - thanks to Burt Leland

## v1.02 release 2008-09-15

### Added

- Introduced **Standard InChI** and **Standard InChI Key** (for detailed information see release notes)
- Added command line option `InpAux` for use in combination with option `STDIO` if the input stream contains structure(s) in AuxInfo form

### Fixed

- Fixed missing "fixed-H" iso segement in case of single removed D(+)
- Fixed bug at reading InChI
- Fixed ["DALKE_BUGS"](https://sourceforge.net/p/inchi/mailman/inchi-discuss/thread/028b01c77cac%24c721f8e0%248801a8c0@xempc3/) (see <https://sourceforge.net/p/inchi/mailman/inchi-discuss/thread/028b01c77cac%24c721f8e0%248801a8c0@xempc3/> for detailed information) - Thanks to A. Dalke
- Fixed bug that leads to missed charge in some cases when /o is present
- Fixed bug of i2i conversion SAbs-->(SRel||Srac)
- Fixed bug of i2i conversion (missed empty /t)
- Fixed bug of i2i conversion (missed repeating /s in FI after F for multi-component case)
- Fixed bug that in some cases (dependent on ordering numbers), moving a charge from terminal H to heavy atom resulted in neutralizing H but not adjusting charge of heavy atom
- Fixed bug which leads for different InChI on atomic permitations for systems containing radical at atom in aromatic ring

## v1.01 release 2005-04

### Added

- New command line option `InChI2InChI` checks input InChI string for significatn syntax errors, may remove layers, result is an InChI string (for detailed information see release notes)
- New command line option `InChI2Struct` converts input InChI into a 0D structure (for detailed information see release notes)
- New command line option `SPXYZ` treats Phosphines as stereogenic
- New command line option `SAsXYZ` treats Arsines as stereogenic
- New command line option `FixSp3bug` or `FB` activates fixes of two known bugs in stereochemical sp<sup>3</sup>(/t) segement

### Fixed

- Several bugs have been fixed, most of the bugs were discussed at [InChI-discuss](https://lists.sourceforge.net/lists/listinfo/inchi-discuss) (<https://lists.sourceforge.net/lists/listinfo/inchi-discuss>)
- Fixed bug that a stereocenter connected by 3 bonds in a 2D structure could be undected if an average bind length is greater than 20
- Fixed bug that a parity of a stereocenter connected by 4 bonds in a 2D structure such that a stereobond exactly overlaps with another single bond may become “undefined” depending on the order of the atoms in the structure
