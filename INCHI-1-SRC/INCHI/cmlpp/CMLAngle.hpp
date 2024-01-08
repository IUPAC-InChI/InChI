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


#ifndef CMLANGLE_HPP
#define CMLANGLE_HPP

#include <string>

#include "XMLUtils.hpp"
#include "CMLBase.hpp"
#include "symbols.hpp"
#include "CMLDocument.hpp"
#include "SaxHandler.hpp"

using namespace std;
namespace CML {

///

class CMLPP_API CMLAngle : public CMLBase {

public:
	//! Constructor
	CMLAngle (CMLDocument* documentPtr = 0);
	CMLAngle (const CMLAngle &other);
	CMLAngle &operator = (CMLAngle &other);

	void endSAX (SaxHandler *saxPtr);

protected: // members
	// current reference to lists of atoms
	string  atomRefs3Array [3];
	double value;
	string units;
};

} //end namespace CML

#endif //CMLANGLE_HPP
