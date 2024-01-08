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


#ifndef CMLATOMARRAY_HPP
#define CMLATOMARRAY_HPP

#include <string>
#include <vector>

#include "CMLBase.hpp"
#include "CMLAtom.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

// forward class declarations

/// A container for a list of CMLAtoms

class CMLPP_API CMLAtomArray : public CMLBase {

	friend class CMLAtomArrayEditor;

public:
	//! Constructor
	CMLAtomArray(CMLDocument* documentPtr = 0);
	CMLAtomArray(const CMLAtomArray& c);
	CMLAtomArray &operator = (CMLAtomArray & other);

	//! Destructor
	virtual ~CMLAtomArray();

	virtual void startSAX(SaxHandler* saxPtr, AttributeVector& atts);
	virtual void endSAX(SaxHandler* saxPtr);
	virtual void writeXML(ostream &os) const;

protected:
	// atom stuff
	unsigned int natoms;

	vector <CMLAtom*> atomPtrVector;

	StringVector idVector;
	StringVector elementTypeVector;
	vector <int> formalChargeVector;
	vector <int> hydrogenCountVector;
	vector <double> x2Vector;
	vector <double> y2Vector;
	vector <double> x3Vector;
	vector <double> y3Vector;
	vector <double> z3Vector;
	vector <double> xFractVector;
	vector <double> yFractVector;
	vector <double> zFractVector;

private:
	void appendAtom (CMLAtom * const atom_p);
};

} //end namespace CML

#endif //CMLATOMARRAY_HPP
