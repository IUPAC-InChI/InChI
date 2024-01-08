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
#include "CMLAngle.hpp"

using namespace std;
namespace CML
{

CMLAngle :: CMLAngle (CMLDocument *documentPtr)
	: CMLBase (documentPtr, C_ANGLE),
		value (0.0),
		units (CH_EMPTY)
{
	atomRefs3Array [0] =
	atomRefs3Array [1] =
	atomRefs3Array [2] = CH_EMPTY;
}


CMLAngle :: CMLAngle (const CMLAngle &other)
	// all of the primitives can be initialised like this
	// likewise anything that doesn't have pointers. So
	// Vector <atom> is ok, Vector <Atom *> is not.
	:	value (other.value),
		units (other.units)
{
	atomRefs3Array [0] = other.atomRefs3Array [0];
	atomRefs3Array [1] = other.atomRefs3Array [1];
	atomRefs3Array [2] = other.atomRefs3Array [2];
}


CMLAngle &CMLAngle :: operator = (CMLAngle &other)
{
	if (this != &other)
 		{
			value = other.value;
			units = other.units;

			// if these were primitives rather than strings
			// we could do memcpy (destArray, srcArray, srcArray.length * sizeof srcArray [0]);

			atomRefs3Array [0] = other.atomRefs3Array [0];
			atomRefs3Array [1] = other.atomRefs3Array [1];
			atomRefs3Array [2] = other.atomRefs3Array [2];
		}

	return *this;
}


void CMLAngle::endSAX(SaxHandler* saxPtr)
{
	CMLBase::endSAX (saxPtr);
	value = atof (saxPtr -> getPCDATA ().c_str ());
}

}	// namespace CML

