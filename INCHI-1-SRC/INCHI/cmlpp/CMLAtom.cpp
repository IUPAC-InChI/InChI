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

#include <cstdlib> // Added 04-25-2010 (to work with gcc 4.3/4.4; thanks to Peter Linstrom)

#include "pragma.hpp"
#include <vector>
#include <cstdio>
#include <sstream>

#include "CMLAtom.hpp"
#include "CMLAtomParity.hpp"
#include "CMLSaxHandler.hpp"
#include "CMLSerializer.hpp"

#ifdef USE_MPATROL
#include "mpatrol.hpp"
#endif

#ifdef _VC6
  #include "vc_mem_leak.hpp"

  #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    #define THIS_FILE __FILE__
  #endif  //#ifdef _DEBUG
#endif  // #ifdef _VC6

using namespace std;
namespace CML {

CMLAtom :: CMLAtom ()
	:	CMLBase ()
{
	init ();
}


CMLAtom :: CMLAtom (CMLDocument *documentPtr)
	: CMLBase (documentPtr, C_ATOM)
{
	init ();
}

CMLAtom :: CMLAtom (const CMLAtom& a)
: elementType (a.elementType),
	elementTypeSet (a.elementTypeSet),
	formalCharge (a.formalCharge),
	formalChargeString (a.formalChargeString),
	formalChargeSet (a.formalChargeSet),
	hydrogenCount (a.hydrogenCount),
	hydrogenCountString (a.hydrogenCountString),
	hydrogenCountSet (a.hydrogenCountSet),
	isotopeNumber (a.isotopeNumber),
	isotopeNumberString (a.isotopeNumberString),
	isotopeNumberSet (a.isotopeNumberSet),
	spinMultiplicity (a.spinMultiplicity),
	spinMultiplicityString (a.spinMultiplicityString),
	spinMultiplicitySet (a.spinMultiplicitySet),
	x2 (a.x2),
	x2String (a.x2String),
	x2Set (a.x2Set),
	y2 (a.y2),
	y2String (a.y2String),
	y2Set (a.y2Set),
	x3 (a.x3),
	x3String (a.x3String),
	x3Set (a.x3Set),
	y3 (a.y3),
	y3String (a.y3String),
	y3Set (a.y3Set),
	z3 (a.z3),
	z3String (a.z3String),
	z3Set (a.z3Set),
	xFract (a.xFract),
	xFractString (a.xFractString),
	xFractSet (a.xFractSet),
	yFract (a.yFract),
	yFractString (a.yFractString),
	yFractSet (a.yFractSet),
	zFract (a.zFract),
	zFractString (a.zFractString),
	zFractSet (a.zFractSet)
{
	if (a.atomParityPtr)
		{
			atomParityPtr = new CMLAtomParity (*a.atomParityPtr);
		}
	else
		{
			atomParityPtr = 0;
		}
}

CMLAtom &CMLAtom :: operator = (CMLAtom &other)
{
	if (this != &other)
		{
			elementType = other.elementType;
			elementTypeSet = other.elementTypeSet;

			formalCharge = other.formalCharge;
			formalChargeString = other.formalChargeString;
			formalChargeSet = other.formalChargeSet;

			hydrogenCount = other.hydrogenCount;
			hydrogenCountString = other.hydrogenCountString;
			hydrogenCountSet = other.hydrogenCountSet;

			isotopeNumber = other.isotopeNumber;
			isotopeNumberString = other.isotopeNumberString;
			isotopeNumberSet = other.isotopeNumberSet;

			spinMultiplicity = other.spinMultiplicity;
			spinMultiplicityString = other.spinMultiplicityString;
			spinMultiplicitySet = other.spinMultiplicitySet;

			x2 = other.x2;
			x2String = other.x2String;
			x2Set = other.x2Set;

			y2 = other.y2;
			y2String = other.y2String;
			y2Set = other.y2Set;

			x3 = other.x3;
			x3String = other.x3String;
			x3Set = other.x3Set;

			y3 = other.y3;
			y3String = other.y3String;
			y3Set = other.y3Set;

			z3 = other.z3;
			z3String = other.z3String;
			z3Set = other.z3Set;

			xFract = other.xFract;
			xFractString = other.xFractString;
			xFractSet = other.xFractSet;

			yFract = other.yFract;
			yFractString = other.yFractString;
			yFractSet = other.yFractSet;

			zFract = other.zFract;
			zFractString = other.zFractString;
			zFractSet = other.zFractSet;

			if (other.atomParityPtr)
				{
					if (atomParityPtr)
						{
							atomParityPtr -> setAtomRefs4 (const_cast <string &> (other.atomParityPtr -> getAtomRefs4 ()));
						}
					else
						{
							atomParityPtr = new CMLAtomParity (* (other.atomParityPtr));
						}
				}
			else
				{
					if (atomParityPtr)
						{
							delete atomParityPtr;
							atomParityPtr = 0;
						}
				}
		}

	return *this;
}




CMLAtom :: ~CMLAtom() {
	//delete atomParityPtr;
}

void CMLAtom :: init() {
	setId(CH_EMPTY);

	x2 =
	y2 =
	x3 =
	y3 =
	z3 =
	xFract =
	yFract =
	zFract = UNSET_DOUBLE;

	formalCharge =
	hydrogenCount =
	isotopeNumber =
	spinMultiplicity = UNSET_INT;

	elementType =
	formalChargeString =
	hydrogenCountString =
	isotopeNumberString =
	spinMultiplicityString = CH_EMPTY;

	atomParityPtr = 0;

	x2Set =
	y2Set =
	x3Set =
	y3Set =
	z3Set =
	xFractSet =
	yFractSet =
	zFractSet =
	formalChargeSet =
	hydrogenCountSet =
	isotopeNumberSet =
	spinMultiplicitySet =
	elementTypeSet = false;
}

bool CMLAtom :: setUntypedAttribute (const string &attName, const string &attVal)
{
	bool result = true;

	if (attName == C_ELEMENTTYPE)
		{
			setElementType (attVal);
		}
	else if (attName == C_FORMALCHARGE)
		{
			setFormalCharge (attVal);
		}
	else if (attName == C_HYDROGENCOUNT)
		{
			setHydrogenCount (attVal);
		}
	else if (attName == C_ISOTOPENUMBER)
		{
			setIsotopeNumber (attVal);
		}
	else if (attName == C_SPINMULTIPLICITY)
		{
			setSpinMultiplicity (attVal);
		}
	else if (attName == C_X2)
		{
			setX2 (attVal);
		}
	else if (attName == C_Y2)
		{
			setY2 (attVal);
		}
	else if (attName == C_X3)
		{
			setX3 (attVal);
		}
	else if (attName == C_Y3)
		{
			setY3 (attVal);
		}
	else if (attName == C_Z3)
		{
			setZ3 (attVal);
		}
	else if (attName == C_XFRACT)
		{
			setXFract (attVal);
		}
	else if (attName == C_YFRACT)
		{
			setYFract (attVal);
		}
	else if (attName == C_ZFRACT)
		{
			setZFract (attVal);
		}
	else
		{
			result = CMLBase :: setUntypedAttribute (attName, attVal);
		}

	return result;
}

void CMLAtom :: setElementType (const string &value)
{
	elementType = value;
	setAttributeValue (C_ELEMENTTYPE, value);
	elementTypeSet = true;
}

const string& CMLAtom :: getElementType () const
{
	return elementType;
}

bool CMLAtom :: elementTypeIsSet() const
{
	return elementTypeSet;
}

int CMLAtom :: getFormalCharge () const
{
	return formalCharge;
}

bool CMLAtom :: formalChargeIsSet () const
{
	return formalChargeSet;
}

int CMLAtom :: getHydrogenCount () const
{
	return hydrogenCount;
}

bool CMLAtom :: hydrogenCountIsSet () const
{
	return hydrogenCountSet;
}/*; extraneous semicolon commented out by DCh 1-18-2005 */

int CMLAtom :: getIsotopeNumber () const
{
	return isotopeNumber;
}

bool CMLAtom :: isotopeNumberIsSet () const
{
	return isotopeNumberSet;
}/*; extraneous semicolon commented out by DCh 1-18-2005 */


int CMLAtom :: getSpinMultiplicity () const
{
	return spinMultiplicity;
}

bool CMLAtom :: spinMultiplicityIsSet () const
{
	return spinMultiplicitySet;
}/*; extraneous semicolon commented out by DCh 1-18-2005 */


// coordinates
double CMLAtom :: getX2 () const
{
	return x2;
}

bool CMLAtom :: x2IsSet () const
{
	return x2Set;
}

double CMLAtom :: getY2 () const
{
	return y2;
}

bool CMLAtom :: y2IsSet () const
{

	return y2Set;
}

double CMLAtom :: getX3 () const
{
	return x3;
}

bool CMLAtom :: x3IsSet () const
{
	return x3Set;
}

double CMLAtom :: getY3 () const
{
	return y3;
}

bool CMLAtom :: y3IsSet () const
{
	return y3Set;
}

double CMLAtom :: getZ3 () const
{
	return z3;
}

bool CMLAtom :: z3IsSet () const
{
	return z3Set;
}

double CMLAtom :: getXFract () const
{
	return xFract;
}

bool CMLAtom :: xFractIsSet() const
{
	return xFractSet;
}

double CMLAtom :: getYFract () const
{
	return yFract;
}

bool CMLAtom :: yFractIsSet() const
{
	return yFractSet;
}

double CMLAtom :: getZFract () const
{
	return zFract;
}

bool CMLAtom :: zFractIsSet() const
{
	return zFractSet;
}

bool CMLAtom :: atomParityIsSet () const
{
	return (atomParityPtr != 0);
}

CMLAtomParity *CMLAtom :: getAtomParity () const
{
	return atomParityPtr;
}

void CMLAtom :: setFormalCharge (int value)
{
	formalChargeString = "";
	ostringstream stm (formalChargeString);
	stm << value;

	formalCharge = value;

	setAttributeValue (C_FORMALCHARGE, value);
}


void CMLAtom :: setFormalCharge (const string &value)
{
	formalChargeString = value;
	formalCharge = atoi (value.c_str ());
	formalChargeSet = true;

	setAttributeValue (C_FORMALCHARGE, value);
}


void CMLAtom :: setHydrogenCount (int value)
{
	hydrogenCountString = "";
	ostringstream stm (hydrogenCountString);
	stm << value;

	hydrogenCount = value;

	setAttributeValue (C_HYDROGENCOUNT, hydrogenCountString);
}


void CMLAtom :: setHydrogenCount (const string &value)
{
	hydrogenCountString = value;
	hydrogenCount = atoi (value.c_str ());
	hydrogenCountSet = true;

	setAttributeValue (C_HYDROGENCOUNT, value);
}


void CMLAtom :: setIsotopeNumber (int value)
{
	isotopeNumberString = "";
	ostringstream stm (isotopeNumberString);
	stm << value;

	isotopeNumber = value;

	setAttributeValue (C_ISOTOPENUMBER, isotopeNumberString);
}


void CMLAtom :: setIsotopeNumber (const string &value)
{
	isotopeNumberString = value;
	isotopeNumber = atoi (value.c_str ());
	isotopeNumberSet = true;

	setAttributeValue (C_ISOTOPENUMBER, value);
}


void CMLAtom :: setSpinMultiplicity (int value)
{
	spinMultiplicityString = "";
	ostringstream stm (spinMultiplicityString);
	stm << value;

	spinMultiplicity = value;

	setAttributeValue (C_SPINMULTIPLICITY, spinMultiplicityString);
}


void CMLAtom :: setSpinMultiplicity (const string &value)
{
	spinMultiplicityString = value;
	spinMultiplicity = atoi (value.c_str ());
	spinMultiplicitySet = true;

	setAttributeValue (C_SPINMULTIPLICITY, value);
}

void CMLAtom :: setX2 (const string &s)
{
	setX2 (atof (s.c_str ()));
}


void CMLAtom :: setX2 (double v)
{
	x2 = v;
	char buffer [20];
	sprintf (buffer, "%10.5f", v);

	x2String = buffer;
	XMLUtils :: trim (x2String);

	setAttributeValue (C_X2, x2String);
	x2Set = true;
}


void CMLAtom :: setY2 (const string &s)
{
	setY2 (atof (s.c_str ()));
}

void CMLAtom :: setY2 (double v)
{
	y2 = v;
	char buffer [20];
	sprintf (buffer, "%10.5f", v);

	y2String = buffer;
	XMLUtils :: trim (y2String);

	setAttributeValue (C_Y2, y2String);
	y2Set = true;
}

void CMLAtom :: setX3 (const string &s)
{
	setX3 (atof (s.c_str ()));
}

void CMLAtom :: setX3 (double v)
{
	x3 = v;
	char buffer [20];
	sprintf (buffer, "%10.5f", v);

	x3String = buffer;
	XMLUtils :: trim (x3String);

	setAttributeValue (C_X3, x3String);
	x3Set = true;
}

void CMLAtom :: setY3 (const string &s)
{
	setY3 (atof (s.c_str ()));
}

void CMLAtom :: setY3 (double v)
{
	y3 = v;
	char buffer [20];
	sprintf (buffer, "%10.5f", v);

	y3String = buffer;
	XMLUtils :: trim (y3String);

	setAttributeValue (C_Y3, y3String);
	y3Set = true;
}

void CMLAtom :: setZ3 (const string &s)
{
	setZ3 (atof (s.c_str ()));
}

void CMLAtom :: setZ3 (double v)
{
	z3 = v;
	char buffer [20];
	sprintf (buffer, "%10.5f", v);

	z3String = buffer;
	XMLUtils :: trim (z3String);

	setAttributeValue (C_Z3, z3String);
	z3Set = true;
}

void CMLAtom :: setXFract (const string &s)
{
	setXFract (atof (s.c_str ()));
}

void CMLAtom :: setXFract (double v)
{
	xFract = v;
	char buffer [20];
	sprintf (buffer, "%10.5f", v);

	xFractString = buffer;
	XMLUtils :: trim (xFractString);

	setAttributeValue (C_XFRACT, xFractString);
	xFractSet = true;
}

void CMLAtom :: setYFract (const string &s)
{
		setYFract (atof (s.c_str ()));
}

void CMLAtom :: setYFract (double v)
{
	yFract = v;
	char buffer [20];
	sprintf (buffer, "%10.5f", v);

	yFractString = buffer;
	XMLUtils :: trim (yFractString);

	setAttributeValue (C_YFRACT, yFractString);
	yFractSet = true;
}

void CMLAtom :: setZFract (const string &s)
{
	setZFract (atof (s.c_str ()));
}

void CMLAtom :: setZFract (double v)
{
	zFract = v;
	char buffer [20];
	sprintf (buffer, "%10.5f", v);

	zFractString = buffer;
	XMLUtils :: trim (zFractString);

	setAttributeValue (C_ZFRACT, zFractString);
	zFractSet = true;
}


void CMLAtom :: setAtomParity (CMLAtomParity * const ptr)
{
	atomParityPtr = ptr;
}


void CMLAtom :: appendChild (Node *node_p)
{
	if (node_p)
		{
			CMLAtomParity *atom_parity_p = dynamic_cast <CMLAtomParity *> (node_p);
			if (atom_parity_p)
				{
					setAtomParity (atom_parity_p);
				}
			(Node :: appendChild (node_p));
		}
}


void CMLAtom :: startSAX (SaxHandler* saxPtr, AttributeVector& atts)
{
	CMLBase :: startSAX (saxPtr, atts);
	CMLSaxHandler *cmlSaxPtr = dynamic_cast <CMLSaxHandler*> (saxPtr);

	if (cmlSaxPtr)
		{
			if (XMLUtils::getAttributeValue(atts, C_FORMALCHARGE) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_X2) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_Y2) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_X3) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_Y3) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_Z3) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_XFRACT) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_YFRACT) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_ZFRACT) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_ELEMENTTYPE) != CH_EMPTY
					)
				{
					cmlSaxPtr->setCmlType(C_CML2);
				}
			else
				{
					cmlSaxPtr->setCmlType(C_CML1);
				}
		}
	else
		{
#ifndef INCHI_LIB
			cerr << "program bug: CMLAtomArray::endSAX(SaxHandler* saxPtr)" << endl;
#endif
        }
}

void CMLAtom :: endSAX (SaxHandler* saxPtr)
{
	//CMLBase :: endSAX (saxPtr);
	CMLSaxHandler* cmlSaxPtr = dynamic_cast <CMLSaxHandler*> (saxPtr);

	if (cmlSaxPtr)
		{
			if (cmlSaxPtr -> getCmlType () == C_CML1)
				{
					convertBuiltinToAttributes (this);
				}
			if (saxPtr -> getDebug ())
				{
#ifndef INCHI_LIB
					cerr << "endAtom: type: " << cmlSaxPtr -> getCmlType () << "; array: "
					<< cmlSaxPtr -> getInputArray () << endl;
#endif
				}

			setUntypedAttributes ();
		}
	else
		{
			XMLUtils :: cmlError("Program bug: CMLAtom :: endSAX(SaxHandler* saxPtr)");
		}
}

void CMLAtom :: writeXML (ostream &os) const
{
	const CMLSerializer* const  cmlSerializerPtr = dynamic_cast <const CMLSerializer* const> (getDocumentPtr()->getSerializerPtr());

	if (cmlSerializerPtr)
		{
			string cmlType = cmlSerializerPtr -> getCmlType ();
			cmlSerializerPtr -> writeStartTagStart (os, C_ATOM);
			os << getAttributeString (C_ID);

	// CML1
			if (cmlType == C_CML1)
				{
					cmlSerializerPtr -> writeStartTagEnd (os);
					outputCML1Builtin (os, C_STRING, C_ELEMENTTYPE, cmlSerializerPtr);
					outputCML1Builtin (os, C_INTEGER, C_FORMALCHARGE, cmlSerializerPtr);
					outputCML1Builtin (os, C_INTEGER, C_SPINMULTIPLICITY, cmlSerializerPtr);
					outputCML1Builtin (os, C_INTEGER, C_ISOTOPENUMBER, cmlSerializerPtr);
					outputCML1Builtin (os, C_INTEGER, C_HYDROGENCOUNT, cmlSerializerPtr);
					outputCML1Builtin (os, C_FLOAT, C_X2, cmlSerializerPtr);
					outputCML1Builtin (os, C_FLOAT, C_Y2, cmlSerializerPtr);
					outputCML1Builtin (os, C_FLOAT, C_X3, cmlSerializerPtr);
					outputCML1Builtin (os, C_FLOAT, C_Y3, cmlSerializerPtr);
					outputCML1Builtin (os, C_FLOAT, C_Z3, cmlSerializerPtr);
					outputCML1Builtin (os, C_FLOAT, C_XFRACT, cmlSerializerPtr);
					outputCML1Builtin (os, C_FLOAT, C_YFRACT, cmlSerializerPtr);
					outputCML1Builtin (os, C_FLOAT, C_ZFRACT, cmlSerializerPtr);


					if (atomParityPtr)
						{
							cmlSerializerPtr -> writeStartTagEnd (os);
							atomParityPtr -> writeXML (os);
						}

					cmlSerializerPtr -> writeEndTag (os, C_ATOM);
				}
			else
				{
	// CML2
					os << getAttributeString (C_ELEMENTTYPE);
					os << getAttributeString (C_FORMALCHARGE);
					os << getAttributeString (C_SPINMULTIPLICITY);
					os << getAttributeString (C_ISOTOPENUMBER);
					os << getAttributeString (C_HYDROGENCOUNT);
					os << getAttributeString (C_X2);
					os << getAttributeString (C_Y2);
					os << getAttributeString (C_X3);
					os << getAttributeString (C_Y3);
					os << getAttributeString (C_Z3);
					os << getAttributeString (C_XFRACT);
					os << getAttributeString (C_YFRACT);
					os << getAttributeString (C_ZFRACT);

					if (atomParityPtr)
						{
							cmlSerializerPtr -> writeStartTagEnd (os);
							atomParityPtr -> writeXML (os);
							cmlSerializerPtr -> writeEndTag (os, C_ATOM);
						}
					else
						{
							cmlSerializerPtr -> writeCombinedTagEnd (os);
						}


				}
		}
	else
		{
#ifndef INCHI_LIB
			cerr << "program bug: CMLAtom :: writeXML" << endl;
#endif
		}
}

}	// namespace CML
