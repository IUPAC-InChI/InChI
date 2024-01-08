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
#include "CMLAtomParity.hpp"

using namespace std;
namespace CML {

CMLAtomParity :: CMLAtomParity ()
{
	init();
}


CMLAtomParity :: CMLAtomParity (CMLDocument *documentPtr)
	: CMLBase (documentPtr, C_ATOMPARITY)
{
	init ();
}


void CMLAtomParity :: init ()
{
	atomRefs4Set = false;
	atomRefs4 = CH_EMPTY;
}


CMLAtomParity :: ~CMLAtomParity ()
{}


const string &CMLAtomParity::getAtomRefs4 () const
{
	return atomRefs4;
}


void CMLAtomParity::setAtomRefs4 (const string &atom_refs)
{
	atomRefs4 = atom_refs;
	atomRefs4Set = true;
}


bool CMLAtomParity :: setUntypedAttribute (const string &attName, const string &attVal)
{
	bool result = true;

	if (attName == C_ATOMREFS4)
		{
			setAtomRefs4 (attVal);
		}
	else
		{
			result = CMLBase :: setUntypedAttribute (attName, attVal);
		}

	return result;
}


void CMLAtomParity :: setAtomRefs4 (string &value1, string &value2, string &value3, string &value4)
{
	atomRefs4 = value1 + " " + value2 + " " + value3 + " " + value4;
	atomRefs4Set = true;
}


bool CMLAtomParity :: atomRefs4ValueIsSet ()
{
	return atomRefs4Set;
}


}

