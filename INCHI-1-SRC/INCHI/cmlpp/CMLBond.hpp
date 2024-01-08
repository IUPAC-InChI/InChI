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


#ifndef CMLBOND_HPP
#define CMLBOND_HPP

#include <string>
#include <vector>

#include "CMLBase.hpp"
#include "CMLBondStereo.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLBond : public CMLBase {

	friend class CMLBondEditor;

public:
	//! Constructor
	CMLBond();
	CMLBond(CMLDocument* documentPtr);
	CMLBond(const CMLBond& b);
	CMLBond &operator = (CMLBond &other);
	virtual ~CMLBond();

	void startSAX(SaxHandler* saxPtr, AttributeVector& atts);
	void endSAX(SaxHandler* saxPtr);
	virtual void writeXML(ostream &os) const;

	virtual void appendChild (Node *node_p);

	virtual bool setUntypedAttribute(const string& attName, const string& attVal);

	string getOrder () const;
	void setOrder (const string& value);
	bool orderIsSet() const;


	CMLBondStereo* getBondStereo () const;
	void setBondStereo (CMLBondStereo * const ptr);
	bool bondStereoIsSet() const;

	void setAtomRefs2(const string& a1, const string& a2);
	void setAtomRefs2(const string& a12);
	const pair <string, string> &getAtomRefs2() const;
	bool atomRefs2IsSet() const;

protected:

	CMLBondStereo* bondStereoPtr;

	pair <string, string> atomRefs2;
	bool atomRefs2Set;

	string order;
	bool orderSet;

	string stereo;
	bool stereoSet;

};

} //end namespace CML

#endif //CMLBOND_HPP
