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


#ifndef CMLBONDSTEREO_HPP
#define CMLBONDSTEREO_HPP

#include <string>

#include "CMLBase.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLBondStereo : public CMLBase {

public:
	//! Constructor
	CMLBondStereo ();
	CMLBondStereo (CMLDocument* documentPtr);
	CMLBondStereo (const CMLBondStereo &b);
	CMLBondStereo &operator = (CMLBondStereo &other);
	//! Destructor
	virtual ~CMLBondStereo ();

	virtual bool setUntypedAttribute(const string& attName, const string& attVal);


	const string &getAtomRefs4() const;
	void setAtomRefs4(const string &value);
  bool atomRefs4ValueIsSet () const;


	void setAtomRefs4(const string * const values_p);
	void setAtomRefs4 (const string &value1, const string &value2, const string &value3, const string &value4);
 	void setAtomRefs4Array (const string &value);
  const string * const getAtomRefs4Array () const;
	bool atomRefs4ArrayValueIsSet () const;


protected:
	string atomRefs4Array [4];
	bool atomRefs4ArraySet;
	string atomRefs4;
	bool atomRefs4Set;

private:
	void init ();

};

} //end namespace CML

#endif //CMLBONDSTEREO_HPP
