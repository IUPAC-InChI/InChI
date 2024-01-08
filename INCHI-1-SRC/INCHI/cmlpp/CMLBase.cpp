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
#include <vector>

#include "CMLBase.hpp"
#include "Document.hpp"
#include "CMLDocument.hpp"

using namespace std;
namespace CML {

const string CMLBase :: EMPTY = CH_EMPTY;
const int CMLBase :: UNSET_INT = -9999999;
const double CMLBase :: UNSET_DOUBLE = -9999999.0;

CMLBase :: CMLBase ()
	: dictRef (EMPTY),
		dictRefSet (false),
		idd (EMPTY),
		idSet (false),
		title (EMPTY),
		titleSet (false)
{}


CMLBase :: CMLBase (const CMLBase &base)
	: dictRef (base.dictRef),
		dictRefSet (base.dictRefSet),
		idd (base.idd),
		idSet (base.idSet),
		title (base.title),
		titleSet (base.titleSet)
{}


CMLBase :: CMLBase (CMLDocument* dPtr, string tagName)
	:	Element (static_cast <Document *> (dPtr), tagName),
		dictRef (EMPTY),
		dictRefSet (false),
		idd (EMPTY),
		idSet (false),
		title (EMPTY),
		titleSet (false)
{}


CMLBase :: ~CMLBase ()
{}


void CMLBase :: setUntypedAttributes ()
{
	int initial_size;
	unsigned int safety_counter = 1000;
	bool loop_flag;

	do
		{
			initial_size = static_cast <int> (atts.size ());

			for (int i = initial_size - 1 ; i >= 0 ; -- i)
				{
					setUntypedAttribute (atts [i].first, atts [i].second);
				}

			loop_flag = (initial_size != static_cast <int> (atts.size ())) && ((-- safety_counter) != 0);
		}
	while (loop_flag);


	if (!safety_counter)
		{
#ifndef INCHI_LIB
			cerr << "CMLBase :: setUntypedAttributes () overflowed. Too many attributes\n";
#endif
		}
}


bool CMLBase :: setUntypedAttribute (const string &attName, const string &attVal)
{
	bool result = true;

	if (attName == C_DICTREF)
		{
			setDictRef (attVal);
		}
	else if (attName == C_ID)
		{
			setId (attVal);
		}
	else if (attName == C_TITLE)
		{
			setTitle (attVal);
		}
	else
		{
			setAttributeValue (attName, attVal);

			result = false;
		}

	return result;
}


bool CMLBase :: dictRefIsSet () const
{
	return dictRefSet;
}


bool CMLBase :: idIsSet () const
{
	return idSet;
}


bool CMLBase :: titleIsSet () const
{
	return titleSet;
}


string CMLBase :: getTitle () const
{
	return (titleSet) ? title : CH_EMPTY;
}


void CMLBase :: setTitle (const string &value)
{
	title = value;
	setAttributeValue (C_TITLE, value);
	titleSet = true;
}


string CMLBase :: getId () const
{
	return (idSet) ? idd : CH_EMPTY;
}


void CMLBase :: setId (const string &value)
{
	idd = value;
	setAttributeValue (C_ID, value);
	idSet = true;
}


string CMLBase :: getDictRef () const
{
	return (dictRefSet) ? dictRef : CH_EMPTY;
}


void CMLBase :: setDictRef (const string &value)
{
	dictRef = value;
	setAttributeValue (C_DICTREF, value);
	dictRefSet = true;
}


// usually overridden and called from overridden method
void CMLBase :: endSAX (SaxHandler *sPtr)
{
	setUntypedAttributes ();
}

void CMLBase::outputCML1Builtin(ostream& os, const string& dataType, const string& attName, const Serializer* const serializerPtr) const
{
	string attValue = getAttributeValue(attName);
	outputCML1Builtin(os, dataType, attName, attValue, serializerPtr);
}

void CMLBase::outputCML1Builtin(ostream& os, const string& dataType, const string& attName,
																const string& attValue, const Serializer* const serializerPtr) const
{
	string s = attValue;
	XMLUtils :: trim (s);

	if (s != CH_EMPTY)
		{
			serializerPtr->writeStartTagStart(os, dataType);
			os << XMLUtils::createAttributeString(C_BUILTIN, attName);
			serializerPtr->writeStartTagEnd(os);
			os << attValue;
			serializerPtr->writeEndTag(os, dataType);
		}
}


void CMLBase::outputCML1Array(ostream& os, const string arrayType, const string attName,
															const Serializer* const serializerPtr, const vector <CMLBase*> &basePtrVector) const
{
	CMLBase* basePtr = basePtrVector[0];
	string attVal = basePtr->getAttributeValue(attName);

	if (attVal != CH_EMPTY)
		{
			serializerPtr->writeStartTagStart(os, arrayType);
			serializerPtr->writeAttribute(os, C_BUILTIN, attName);
			serializerPtr->writeStartTagEnd(os);

			const unsigned int size = basePtrVector.size ();

			for (unsigned int i = 0; i < size; ++ i)
				{
					CMLBase* basePtr = basePtrVector[i];

					if (i != 0)
						{
							os << " ";
						}

					os << basePtr->getAttributeValue(attName);
				}

			serializerPtr->writeEndTag(os, arrayType);
		}
}


void CMLBase::outputCML2Attribute(ostream& os, const string &attName, const vector <CMLBase*> &basePtrVector) const
{
	if (basePtrVector.size() == 0)
		{
#ifndef INCHI_LIB
			cerr << "<!-- no child nodes, suspicious? -->" << endl;
#endif
			return;
		}

	CMLBase* basePtr = basePtrVector[0];
	string attVal = basePtr->getAttributeValue(attName);
	if (attVal != CH_EMPTY)
		{
			os << " " << attName << "=\"";
			const unsigned int size = basePtrVector.size();

			for (unsigned int i = 0; i < size; ++ i)
				{
					basePtr = basePtrVector[i];
					if (i != 0)
						{
							os << " ";
						}
					os << basePtr->getAttributeValue(attName);
				}

			os << "\"";
		}
}

void CMLBase::getAtomRefs(StringVector::size_type size, StringVector &v, const string &atomRefString)
{
	StringVector sv;
	string s = atomRefString + CH_SPACE;
	XMLUtils::tokenize(sv, atomRefString, CH_SPACE);

	if (sv.size() != size)
		{
			string ss = "unexpected size for atomRefs attribute: ";
			ss += sv.size();
			ss += CH_SLASH;
			ss += size;
			XMLUtils::cmlError(ss);
			return;
		}

	v = sv;
}

void CMLBase::convertBuiltinToAttributes(CMLBase* basePtr)
{
	vector <Node*> cv = basePtr->getChildVector();
	const unsigned int size = cv.size();
	string atomRefs2 = CH_EMPTY;

	for (unsigned int i = 0; i < size; ++ i)
		{
			string name = cv[i]->getNodeName();
			if (name != CH_EMPTY && name.substr(0,1) != "#")
				{
					Element* elPtr = dynamic_cast <Element*> (cv[i]);
					if (elPtr)
						{
							string builtel = elPtr->getNodeName();
							if (builtel == C_STRING || builtel == C_FLOAT || builtel == C_INTEGER)
								{
									string builtin = elPtr->getAttributeValue(C_BUILTIN);
									string pcd = elPtr->getPCDATA();
									string attval = basePtr->getAttributeValue(builtin);
									if (builtin == C_ATOMREF)
										{
											if (atomRefs2 == CH_EMPTY)
												{
													atomRefs2 = pcd;
												}
											else
												{
													atomRefs2 += " ";
													atomRefs2 += pcd;
													basePtr->setAttributeValue(C_ATOMREFS2, atomRefs2);
												}
										}
									else
										{
											attval = pcd;
											basePtr->setAttributeValue(builtin, pcd);
										}
								}
						}
					else
						{
#ifndef INCHI_LIB
							cerr << "bug at CMLBase::convertBuiltinToAttributes" << endl;
#endif
						}
				}
		}

	clearChildVector();
}

void CMLBase::convertBuiltinArrayToVector(CMLBase* basePtr, string& builtin, string& array)
{
	string name = basePtr->getNodeName();
	if (name != CH_EMPTY && name.substr(0,1) != "#")
		{
			if (name == C_STRINGARRAY || name == C_FLOATARRAY || name == C_INTEGERARRAY)
				{
					builtin = basePtr->getAttributeValue(C_BUILTIN);
					array = basePtr->getPCDATA();
				}
		}
}

}	// namespace CML
