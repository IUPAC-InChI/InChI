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


#ifndef CMLBONDARRAY_HPP
#define CMLBONDARRAY_HPP

#include <string>
#include <vector>

#include "CMLBase.hpp"
#include "CMLBond.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLMolecule;	// forward class declaration

class CMLPP_API CMLBondArray : public CMLBase {

	friend class CMLBondArrayEditor;

public:

	//! Constructor
	CMLBondArray();
	CMLBondArray(CMLDocument* documentPtr);
	CMLBondArray(const CMLBondArray& b);
	CMLBondArray &operator = (CMLBondArray & other);

	//! Destructor
	virtual ~CMLBondArray();

	void endSAX(SaxHandler* saxPtr);
	void writeXML(ostream &os) const;

  bool getAtomRefs2Array(const vector <CMLBond*>& bondPtrVector, pair <string, string> **atom_refs2_pp) const;

protected:
	static CMLMolecule* molPtr;
	vector <CMLBond*> bondPtrVector;

	unsigned int nbonds;

	StringVector idVector;
	StringVector atomRef1Vector;
	StringVector atomRef2Vector;
	StringVector orderVector;
	StringVector stereoVector;

private:
	void appendBond (CMLBond * const bond_p);
	void init();
};

} //end namespace CML

#endif //CMLBONDARRAY_HPP
