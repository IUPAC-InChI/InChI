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


#ifndef CMLATOM_HPP
#define CMLATOM_HPP

#include <string>
#include <vector>

#include "CMLBase.hpp"
#include "CMLSaxHandler.hpp"
#include "symbols.hpp"
#include "CMLDocument.hpp"
#include "CMLAtomParity.hpp"

using namespace std;
namespace CML {

/** Represents an atom
	*/
class CMLPP_API CMLAtom : public CMLBase {

	friend class CMLAtomEditor;

public:
	//! Constructors
	CMLAtom ();
	CMLAtom (CMLDocument* documentPtr);
	CMLAtom (const CMLAtom& a);
	CMLAtom &operator = (CMLAtom &other);
	//! Destructor
	virtual ~CMLAtom();

	virtual void startSAX(SaxHandler* saxPtr, AttributeVector& atts);
	virtual void endSAX(SaxHandler* saxPtr);
	virtual void writeXML(ostream &os) const;

	virtual bool setUntypedAttribute(const string& attName, const string& attVal);


	virtual void appendChild (Node *nodePtr);


	/**
	* Element type.
	*
	* The elementType, defined by a controlled list of elements. Only describes the charge on the nucleus. Do NOT confuse with atomType. Almost mandatory
	* @return string representation of the element type
	*/
	const string& getElementType () const;
	void setElementType (const string& value);
	bool elementTypeIsSet() const;

	/**
	* The formalCharge on the atom (in electrons).
	*
	*	NOT the calculated charge or oxidation state. No formal default, but its absence implies 0. It may be good practice to include it explicitly.
	* generateGetPrimitive
	* gets variable from string representation
	* @throws CMLException
	* @return int
	*/
	int getFormalCharge () const;
	void setFormalCharge (const int value);
	void setFormalCharge (const string& value);
	bool formalChargeIsSet() const;

	int getHydrogenCount () const;
	void setHydrogenCount (const int value);
	void setHydrogenCount (const string& value);
	bool hydrogenCountIsSet() const;


	int getIsotopeNumber () const;
	void setIsotopeNumber (const int value);
	void setIsotopeNumber (const string& value);
	bool isotopeNumberIsSet() const;

	int getSpinMultiplicity () const;
	void setSpinMultiplicity (const int value);
	void setSpinMultiplicity (const string& value);
	bool spinMultiplicityIsSet() const;


	// coordinates
	double getX2 () const;
	void setX2 (const double value);
	void setX2 (const string& value);
	bool x2IsSet() const;

	double getY2 () const;
	void setY2 (const double value);
	void setY2 (const string& value);
	bool y2IsSet() const;

	double getX3 () const;
	void setX3 (const double value);
	void setX3 (const string& value);
	bool x3IsSet() const;

	double getY3 () const;
	void setY3 (const double value);
	void setY3 (const string& value);
	bool y3IsSet() const;

	double getZ3 () const;
	void setZ3 (const double value);
	void setZ3 (const string& value);
	bool z3IsSet() const;

	double getXFract () const;
	void setXFract (const double value);
	void setXFract (const string& value);
	bool xFractIsSet() const;

	double getYFract () const;
	void setYFract (const double value);
	void setYFract (const string& value);
	bool yFractIsSet() const;

	double getZFract () const;
	void setZFract (const double value);
	void setZFract (const string& value);
	bool zFractIsSet() const;

	CMLAtomParity* getAtomParity () const;
	void setAtomParity (CMLAtomParity * const ptr);
	bool atomParityIsSet() const;

protected:
	string elementType;
	bool elementTypeSet;

	int formalCharge;
	string formalChargeString;
	bool formalChargeSet;

	int hydrogenCount;
	string hydrogenCountString;
	bool hydrogenCountSet;

	int isotopeNumber;
	string isotopeNumberString;
	bool isotopeNumberSet;

	int spinMultiplicity;
	string spinMultiplicityString;
	bool spinMultiplicitySet;

	double x2;
	string x2String;
	bool x2Set;

	double y2;
	string y2String;
	bool y2Set;

	double x3;
	string x3String;
	bool x3Set;

	double y3;
	string y3String;
	bool y3Set;

	double z3;
	string z3String;
	bool z3Set;

	double xFract;
	string xFractString;
	bool xFractSet;

	double yFract;
	string yFractString;
	bool yFractSet;

	double zFract;
	string zFractString;
	bool zFractSet;

	CMLAtomParity* atomParityPtr;

private:
	void init();

};

} //end namespace CML

#endif //CMLATOM_HPP
