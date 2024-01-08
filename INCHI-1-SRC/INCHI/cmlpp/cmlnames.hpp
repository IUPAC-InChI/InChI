/*
 * CMLReader for InChI
 * Developed in collaboration with NIST and IUPAC
 * Version 1.0
 * Software version 1.0
 * 2004
 * Developed by Peter Murray-Rust and Simon (Billy) Tyrrell
 * Unilever Centre for Molecular Sciences Informatics
 * Department of Chemistry, University of Cambridge
 * Cambridge, CB2 1EW, UK
 *
 * CMLReader is free software 
 * you can redistribute this software and/or modify it under the terms of
 * the GNU Lesser General Public License as published by the Free Software
 * Foundation:
 * http://www.opensource.org/licenses/lgpl-license.php
 */


#ifndef CMLNAMES_H
#define CMLNAMES_H

#include <string>

using namespace std;
namespace CML {

static const string CML1_NAMESPACE   = "http://www.xml-cml.org/dtd/cml_1_0_1.dtd";
static const string CML2_NAMESPACE   = "http://www.xml-cml.org/schema/cml2/core";
static const string STMML_NAMESPACE  = "http://www.xml-cml.org/schema/stmml";
static const string C_PREFIX         = "cml";

static const string C_2D = "2D";
static const string C_3D = "3D";


static const string C_ATOMID           = "atomID";
static const string C_ATOMREF          = "atomRef";				///< CML1 only
static const string C_ATOMREF1         = "atomRef1";			///< CML2 CMLBondArray only
static const string C_ATOMREF2         = "atomRef2";			///< CML2 CMLBondArray only
static const string C_ATOMREFS         = "atomRefs";			///< CML2 only. General list of atoms
//static const string C_ATOMREFS1        = "atomRefs1";			///< dunno
static const string C_ATOMREFS2        = "atomRefs2";			///< CML2 CMLBond only
static const string C_ATOMREFS3        = "atomRefs3";			///< CML2 CMLAngle only
static const string C_ATOMREFS4        = "atomRefs4";			///< CML2 CMLTorsion, CMLAtomParity+CMLBondStereo only
static const string C_BONDID           = "bondID";
static const string C_BUILTIN          = "builtin";
static const string C_CONVENTION       = "convention";
static const string C_CONTENT          = "content";
static const string C_DATATYPE         = "dataType";
static const string C_DICTREF          = "dictRef";
static const string C_FORMALCHARGE     = "formalCharge";
static const string C_ELEMENTTYPE      = "elementType";
static const string C_HYDROGENCOUNT    = "hydrogenCount";
static const string C_ID               = "id";
static const string C_ISOTOPENUMBER    = "isotopeNumber";
static const string C_NAME             = "name";
static const string C_OCCUPANCY        = "occupancy";
static const string C_ORDER            = "order";
static const string C_POINTGROUP       = "pointgroup";
static const string C_SPACEGROUP       = "spacegroup";
static const string C_SPINMULTIPLICITY = "spinMultiplicity";
static const string C_TITLE            = "title";
static const string C_UNITS            = "units";
static const string C_X2               = "x2";
static const string C_Y2               = "y2";
static const string C_X3               = "x3";
static const string C_Y3               = "y3";
static const string C_Z3               = "z3";
static const string C_XFRACT           = "xFract";
static const string C_YFRACT           = "yFract";
static const string C_ZFRACT           = "zFract";
static const string C_XY2              = "xy2";
static const string C_XYZ3             = "xyz3";
static const string C_XYZFRACT         = "xyzFract";

// metadata
static const string DC_DESCRIPTION     = "dc:description";
static const string DC_IDENTIFIER      = "dc:identifier";
static const string DC_CONTENT         = "dc:content";
static const string DC_RIGHTS          = "dc:rights";
static const string DC_TYPE            = "dc:type";
static const string DC_CONTRIBUTOR     = "dc:contributor";
static const string DC_CREATOR         = "dc:creator";
static const string DC_DATE            = "dc:date";

static const string CMLM_STRUCTURE     = "cmlm:structure";

// CML and STMML elements
static const string C_ANGLE            = "angle";
static const string C_ARRAY            = "array";
static const string C_ATOM             = "atom";
static const string C_ATOMARRAY        = "atomArray";
static const string C_ATOMPARITY       = "atomParity";
static const string C_BOND             = "bond";
static const string C_BONDARRAY        = "bondArray";
static const string C_BONDSTEREO       = "bondStereo";
static const string C_CML              = "cml";
static const string C_COORDINATE2      = "coordinate2";
static const string C_COORDINATE3      = "coordinate3";
static const string C_CRYSTAL          = "crystal";
static const string C_ELECTRON         = "electron";
static const string C_FEATURE          = "feature";
static const string C_FLOAT            = "float";
static const string C_FLOATARRAY       = "floatArray";
static const string C_FLOATMATRIX      = "floatMatrix";
static const string C_FORMULA          = "formula";
static const string C_INTEGER          = "integer";
static const string C_INTEGERARRAY     = "integerArray";
static const string C_LENGTH           = "length";
static const string C_MATRIX           = "matrix";
static const string C_METADATA         = "metadata";
static const string C_METADATALIST     = "metadataList";
static const string C_MOLECULE         = "molecule";
static const string C_REACTION         = "reaction";
static const string C_SCALAR           = "scalar";
static const string C_SEQUENCE         = "sequence";
static const string C_STEREO           = "stereo";
static const string C_STRING           = "string";
static const string C_STRINGARRAY      = "stringArray";
static const string C_SYMMETRY         = "symmetry";
static const string C_TORSION          = "torsion";


static const string C_CML1             = "CML1";
static const string C_CML2             = "CML2";

static const string CMLPP_VERSION      = "V1.0";

static const string C_ANGSTROM         = "angstrom";
static const string C_DEGREES          = "degrees";

static const string S_EMPTY            = "";
}

#endif // CMLNAMES_H

