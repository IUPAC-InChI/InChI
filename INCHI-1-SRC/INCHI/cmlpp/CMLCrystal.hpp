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


#ifndef CMLCRYSTAL_HPP
#define CMLCRYSTAL_HPP

#include <string>

#include "CMLBase.hpp"
#include "CMLSymmetry.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {


class CMLPP_API CMLCrystal : public CMLBase {

public:

	//! Constructor
	CMLCrystal();
	CMLCrystal(CMLDocument* documentPtr);
	CMLCrystal(const CMLCrystal& c);
	CMLCrystal &operator = (CMLCrystal & other);
	//! Destructor
	virtual ~CMLCrystal();

	void setCellParam(const unsigned int num, const string& value);
	void setCellParam(const unsigned int num, double value);

	double getCellParam(const unsigned int num) const;

protected:
	CMLSymmetry* symmetryPtr;
	double cellParam[6];
	string spacegroup;
	string pointgroup;

private:
	void init();
};

} //end namespace CML

#endif //CMLCRYSTAL_HPP
