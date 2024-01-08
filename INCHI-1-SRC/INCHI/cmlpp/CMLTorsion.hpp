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


#ifndef CMLTORSION_HPP
#define CMLTORSION_HPP

#include <string>

#include "CMLBase.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLTorsion : public CMLBase {

public:
	//! Constructor
	CMLTorsion();
	CMLTorsion(CMLDocument* documentPtr);
	CMLTorsion(const CMLTorsion& t);
	//! Destructor
	virtual ~CMLTorsion();

protected:
	string atomRefs4Array [4];
	string units;
	double value;

private:
	void init();
};

} //end namespace CML

#endif //CMLTORSION_HPP
