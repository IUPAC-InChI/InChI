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


#ifndef CMLATOMPARITY_HPP
#define CMLATOMPARITY_HPP

#include <string>
#include <vector>

#include "CMLBase.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLAtomParity : public CMLBase {

public:
	//! Constructor
	CMLAtomParity ();
	CMLAtomParity (CMLDocument* documentPtr);

	//! Destructor
	virtual ~CMLAtomParity ();

	virtual bool setUntypedAttribute (const string& attName, const string& attVal);

	const string &getAtomRefs4 () const;
	void setAtomRefs4 (const string &atom_refs);

	const string &getAtomRefs4StringList () const;
	void setAtomRefs4 (string *atom_refs_p);

	void setAtomRefs4 (string &value1, string &value2, string &value3, string &value4);
	bool atomRefs4ValueIsSet ();


private:
	void init ();

	string atomRefs4;
	bool atomRefs4Set;

};

} //end namespace CML

#endif //CMLATOMPARITY_HPP
