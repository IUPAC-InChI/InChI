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


#ifndef CMLMOLECULE_HPP
#define CMLMOLECULE_HPP


#include "CMLBase.hpp"
#include "CMLAtomArray.hpp"
#include "CMLBondArray.hpp"

#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLMolecule : public CMLBase {

	friend class CMLMoleculeEditor;

public:
	//! Constructor
	CMLMolecule(CMLDocument* documentPtr = 0);

	//! Destructor
	virtual ~CMLMolecule();


};

} //end namespace CML

#endif //CMLMOLECULE_HPP
