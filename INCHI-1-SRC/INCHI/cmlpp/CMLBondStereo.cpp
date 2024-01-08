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
#include "CMLBondStereo.hpp"
#include "conversions.hpp"

using namespace std;
namespace CML {

CMLBondStereo :: CMLBondStereo ()
{
  init ();
}

CMLBondStereo :: CMLBondStereo (CMLDocument* documentPtr)
	: CMLBase (documentPtr, C_BONDSTEREO)
{
	init();
}

CMLBondStereo :: CMLBondStereo (const CMLBondStereo& b)
	:	atomRefs4ArraySet (b.atomRefs4ArraySet),
		atomRefs4 (b.atomRefs4),
		atomRefs4Set (b.atomRefs4Set)
{
	if (atomRefs4ArraySet)
		{
			atomRefs4Array [0] = b.atomRefs4Array [0];
			atomRefs4Array [1] = b.atomRefs4Array [1];
			atomRefs4Array [2] = b.atomRefs4Array [2];
			atomRefs4Array [3] = b.atomRefs4Array [3];
		}
}

CMLBondStereo &CMLBondStereo :: operator = (CMLBondStereo &other)
{
	if (this != &other)
		{
			atomRefs4Array [0] = other.atomRefs4Array [0];
			atomRefs4Array [1] = other.atomRefs4Array [1];
			atomRefs4Array [2] = other.atomRefs4Array [2];
			atomRefs4Array [3] = other.atomRefs4Array [3];
			atomRefs4 = other.atomRefs4;

			atomRefs4Set = other.atomRefs4Set;
			atomRefs4ArraySet = other.atomRefs4ArraySet;
		}

	return *this;
}


CMLBondStereo :: ~CMLBondStereo ()
{}


void CMLBondStereo :: init ()
{
	atomRefs4Array [0] =
	atomRefs4Array [1] =
	atomRefs4Array [2] =
	atomRefs4Array [3] =
	atomRefs4 = CH_EMPTY;

	atomRefs4Set =
	atomRefs4ArraySet = false;
}


void CMLBondStereo :: setAtomRefs4 (const string &value)
{
	setAtomRefs4Array (value);
}

const string &CMLBondStereo :: getAtomRefs4 () const
{
	return atomRefs4;
}

bool CMLBondStereo :: atomRefs4ValueIsSet () const
{
	return atomRefs4Set;
}


void CMLBondStereo :: setAtomRefs4 (const string *values_p)
{
	const string s1 = *values_p;
	const string s2 = * (++ values_p);
	const string s3 = * (++ values_p);
	const string s4 = * (++ values_p);

	setAtomRefs4 (s1, s2, s3, s4);
}

const string * const CMLBondStereo :: getAtomRefs4Array () const
{
	return atomRefs4Array;
}



void CMLBondStereo :: setAtomRefs4Array (const string &s)
{
	char **values_pp;

	const int num_tokens = tokenizeString (s, " ", &values_pp);

	if (num_tokens == 4)
		{
			const string s1 = *values_pp;
			const string s2 = * (values_pp + 1);
			const string s3 = * (values_pp + 2);
			const string s4 = * (values_pp + 3);

			setAtomRefs4 (s1, s2, s3, s4);
		}
	else
		{
			cerr << "Not enough BondSterao atomRefs " << s << endl;
		}


	for (int i = 0; i < num_tokens; ++ i)
		{
			delete [] (* (values_pp + i));
		}
	delete [] values_pp;
}


void CMLBondStereo :: setAtomRefs4 (const string &value1, const string &value2, const string &value3, const string &value4)
{
	atomRefs4Array [0] = value1;
	atomRefs4Array [1] = value2;
	atomRefs4Array [2] = value3;
	atomRefs4Array [3] = value4;

	atomRefs4 = value1 + " " + value2 + " " + value3 + " " + value4;

	atomRefs4Set =
	atomRefs4ArraySet = true;
}

bool CMLBondStereo :: atomRefs4ArrayValueIsSet () const
{
	return atomRefs4ArraySet;
}


bool CMLBondStereo :: setUntypedAttribute (const string &attName, const string &attVal)
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




}	// namespace CML

