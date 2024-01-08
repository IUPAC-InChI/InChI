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


#ifndef CMLDOCUMENT_HPP
#define CMLDOCUMENT_HPP

#include <string>
#include <vector>

#include "Document.hpp"
#include "Text.hpp"
#include "symbols.hpp"

/* document class (== W3C DOM) */
using namespace std;
namespace CML {

class CMLAngle;
class CMLArray;
class CMLAtom;
class CMLAtomArray;
class CMLAtomParity;
class CMLBase;
class CMLBond;
class CMLBondArray;
class CMLBondStereo;
class CMLCml;
class CMLCrystal;
class CMLFormula;
class CMLLength;
class CMLMetadata;
class CMLMetadataList;
class CMLMolecule;
class CMLScalar;
class CMLSymmetry;
class CMLTorsion;


class CMLPP_API CMLDocument : public Document {


public:
	//! Constructor
	CMLDocument();

	CMLDocument(InputWrapper& input);

	CMLDocument(const CMLDocument& d);

	virtual ~CMLDocument();

	virtual Serializer *getSerializerPtr() const;

	virtual bool readXML(InputWrapper& input);
	virtual bool readXML(InputWrapper& input, bool debug);



	//    void writeXML(ostream &os);

	/* factory methods */
	CMLAngle* createCMLAngle();

	CMLArray* createCMLArray();

	CMLAtom* createCMLAtom();

	CMLAtomArray* createCMLAtomArray();

	CMLAtomParity* createCMLAtomParity();

	CMLBase* createCMLBase(string tag);

	CMLBond* createCMLBond();

	CMLBondArray* createCMLBondArray();

	CMLBondStereo* createCMLBondStereo();

	CMLCml* createCMLCml();

	CMLCrystal* createCMLCrystal();

	CMLFormula* createCMLFormula();

	CMLLength* createCMLLength();

	CMLMetadata* createCMLMetadata();

	CMLMetadataList* createCMLMetadataList();

	CMLMolecule* createCMLMolecule();

	CMLScalar* createCMLScalar();

	CMLSymmetry* createCMLSymmetry();

	CMLTorsion* createCMLTorsion();

	Text* createText(const string &s);
	Text* createText();

protected:
	virtual bool fixRootElements (Node *node_p);


private:
	void init();
};

} //end namespace CML

#endif //CMLDOCUMENT_HPP
