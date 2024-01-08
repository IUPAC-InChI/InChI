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


#ifndef CMLSCALAR_HPP
#define CMLSCALAR_HPP

#include <string>

#include "CMLBase.hpp"
#include "SaxHandler.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLScalar : public CMLBase {

public:
	//! Constructor
	CMLScalar();
	CMLScalar(CMLDocument* documentPtr);
	//! Destructor
	virtual ~CMLScalar();

	virtual void endSAX(SaxHandler* saxPtr);

protected:
	string units;
	string dataType;

private:
	void init();


};

} //end namespace CML

#endif //CMLSCALAR_HPP
