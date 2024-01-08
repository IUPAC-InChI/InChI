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


#ifndef CMLLENGTH_HPP
#define CMLLENGTH_HPP

#include <string>

#include "CMLBase.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLLength : public CMLBase {

public:
	//! Constructor
	CMLLength();
	CMLLength(CMLDocument* documentPtr);
	CMLLength(const CMLLength& l);
	//! Destructor
	virtual ~CMLLength();

protected:
	StringVector atomRefs2Vector;
	string units;
	double value;

private:
	void init();

};

} //end namespace CML

#endif //CMLLENGTH_HPP
