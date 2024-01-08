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


#ifndef CMLFORMULA_HPP
#define CMLFORMULA_HPP

#include "CMLBase.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLFormula : public CMLBase {

public:
	//! Constructor
	CMLFormula();
	CMLFormula(CMLDocument* documentPtr);
	CMLFormula(const CMLFormula& f);
	//! Destructor
	virtual ~CMLFormula();

private:
	void init();
};

} //end namespace CML

#endif //CMLFORMULA_HPP
