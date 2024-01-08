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


#include "pragma.hpp"
#include "CMLBond.hpp"
#include "CMLBondStereo.hpp"
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

CMLBond :: CMLBond ()
	:	bondStereoPtr (0),
		atomRefs2 (CH_EMPTY, CH_EMPTY),
		atomRefs2Set (false),
		order (CH_EMPTY),
		orderSet (false),
		stereo (CH_EMPTY),
		stereoSet (false)
{}

CMLBond :: CMLBond (CMLDocument *documentPtr)
	:	CMLBase (documentPtr, C_BOND),
		bondStereoPtr (0),
		atomRefs2 (CH_EMPTY, CH_EMPTY),
		atomRefs2Set (false),
		order (CH_EMPTY),
		orderSet (false),
		stereo (CH_EMPTY),
		stereoSet (false)
{}

CMLBond::CMLBond(const CMLBond& b)
	:	atomRefs2 (b.atomRefs2),
		atomRefs2Set (b.atomRefs2Set),
		order (b.order),
		orderSet (b.orderSet),
		stereo (b.stereo),
		stereoSet (b.stereoSet)
{
	if (b.bondStereoPtr)
		{
			bondStereoPtr = new CMLBondStereo (* (b.bondStereoPtr));
		}
	else
		{
			bondStereoPtr = 0;
		}
}


CMLBond &CMLBond :: operator = (CMLBond &other)
{
	if (this != &other)
		{
			atomRefs2 = other.atomRefs2;
			atomRefs2Set = other.atomRefs2Set;
			order = other.order;
			orderSet  = other.orderSet;
			stereo = other.stereo;
			stereoSet  = other.stereoSet;

			if (bondStereoPtr)
				{
					if (other.bondStereoPtr)
						{
							*bondStereoPtr = * (other.bondStereoPtr);
						}
					else
						{
							delete bondStereoPtr;
							bondStereoPtr = 0;
						}
				}
			else
				{
					if (other.bondStereoPtr)
						{
							bondStereoPtr = new CMLBondStereo (* (other.bondStereoPtr));
						}
				}
		}

	return *this;
}


CMLBond :: ~CMLBond ()
{
	//delete bondStereoPtr;
}

bool CMLBond::setUntypedAttribute(const string& attName, const string& attVal)
{
		bool result = true;

		if (attName == C_ORDER)
			{
				setOrder(attVal);
			}
		else if (attName == C_ATOMREFS2)
			{
				setAtomRefs2(attVal);
			}
		else
			{
				result = CMLBase :: setUntypedAttribute (attName, attVal);
			}


		return result;
}


bool CMLBond::orderIsSet() const
{
	return orderSet;
}


string CMLBond::getOrder () const
{
	return (orderSet) ? order : CH_EMPTY;
}


void CMLBond::setOrder (const string& value)
{
	order = value;
	setAttributeValue(C_ORDER, value);
	orderSet = true;
}

/// ATOM REFS 2

bool CMLBond::atomRefs2IsSet() const
{
	return atomRefs2Set;
}

const pair <string, string> &CMLBond::getAtomRefs2() const
{
	return atomRefs2;
}

void CMLBond::setAtomRefs2(const string& s)
{
	if (s != CH_EMPTY)
		{
			StringVector v;
			XMLUtils::tokenizeStringToStringArray(v, 2, s);

			setAttributeValue (C_ATOMREFS2, s);
			atomRefs2.first = v [0];
			atomRefs2.second = v [1];
//

			setAttributeValue (C_ATOMREF, v [0]);
			setAttributeValue (C_ATOMREF, v [1]);

			atomRefs2Set = true;
		}
}


void CMLBond::setAtomRefs2(const string& a1, const string& a2)
{
	atomRefs2.first = a1;
	atomRefs2.second = a2;

	string s = a1;
	s.append (" ");
	s.append (a2);

	setAttributeValue (C_ATOMREFS2, s);

	atomRefs2Set = true;
}

CMLBondStereo *CMLBond :: getBondStereo () const
{
	return bondStereoPtr;
}


void CMLBond :: setBondStereo (CMLBondStereo * const ptr)
{
	bondStereoPtr = ptr;
}


bool CMLBond :: bondStereoIsSet () const
{
	return (bondStereoPtr != 0);
}


void CMLBond :: appendChild (Node *node_p)
{
	if (node_p)
		{
			CMLBondStereo *bond_stereo_p = dynamic_cast <CMLBondStereo *> (node_p);
			if (bond_stereo_p)
				{
					setBondStereo (bond_stereo_p);
				}
			(Node :: appendChild (node_p));
		}
}

/// SAX

void CMLBond::startSAX(SaxHandler* saxPtr, AttributeVector& atts)
{
	CMLBase::startSAX(saxPtr, atts);
	CMLSaxHandler* cmlSaxPtr = dynamic_cast <CMLSaxHandler*> (saxPtr);

	if (cmlSaxPtr)
		{
			if (XMLUtils::getAttributeValue(atts, C_ATOMREFS2) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_ORDER) != CH_EMPTY ||
					XMLUtils::getAttributeValue(atts, C_STEREO) != CH_EMPTY)
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
			cerr << "program bug: CMLBond::endSAX(SaxHandler* saxPtr)" << endl;
#endif
		}
}


void CMLBond::endSAX(SaxHandler* saxPtr)
{
	//CMLBase::endSAX(saxPtr);
	CMLSaxHandler* cmlSaxPtr = dynamic_cast <CMLSaxHandler*> (saxPtr);
	if (cmlSaxPtr)
		{

			if (cmlSaxPtr->getCmlType() == C_CML1)
				{
					convertBuiltinToAttributes(this);
				}
			setUntypedAttributes();
		}
	else
		{
#ifndef INCHI_LIB
			cerr << "program bug: CMLBond::endSAX(SaxHandler* saxPtr)" << endl;
#endif
		}
}

void CMLBond::writeXML(ostream &os) const
{
	const CMLSerializer* const cmlSerializerPtr = dynamic_cast <const CMLSerializer* const> (getDocumentPtr()->getSerializerPtr());
	if (cmlSerializerPtr)
		{

			string cmlType = cmlSerializerPtr->getCmlType();
			cmlSerializerPtr->writeStartTagStart(os, C_BOND);
			os << getAttributeString(C_ID);

			if (cmlType == C_CML1)
				{
					// CML1
					cmlSerializerPtr->writeStartTagEnd(os);
					outputCML1Builtin(os, C_STRING, C_ID, cmlSerializerPtr);
					//setAtomRefs2(getAttributeValue(C_ATOMREFS2));
					outputCML1Builtin(os, C_STRING, C_ATOMREF, atomRefs2.first, cmlSerializerPtr);
					outputCML1Builtin(os, C_STRING, C_ATOMREF, atomRefs2.second, cmlSerializerPtr);
					outputCML1Builtin(os, C_STRING, C_ORDER, cmlSerializerPtr);

					if (bondStereoPtr)
						{
							cmlSerializerPtr -> writeStartTagEnd (os);
							bondStereoPtr -> writeXML (os);
						}

					cmlSerializerPtr -> writeEndTag (os, C_BOND);
		 		}
		 	else
		 		{
					// CML2
					os << getAttributeString(C_ID);
					os << getAttributeString(C_ATOMREFS2);
					os << getAttributeString(C_ORDER);

					if (bondStereoPtr)
						{
							cmlSerializerPtr -> writeStartTagEnd (os);
							bondStereoPtr -> writeXML (os);
							cmlSerializerPtr -> writeEndTag (os, C_BOND);
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
			cerr << "program bug: CMLBond::writeXML" << endl;
#endif
		}
}


}	// namespace CML

