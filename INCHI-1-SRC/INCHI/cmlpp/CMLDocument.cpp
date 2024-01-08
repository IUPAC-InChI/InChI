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


#include "pragma.hpp"
#include "CMLDocument.hpp"

#include "CMLAngle.hpp"
#include "CMLArray.hpp"
#include "CMLAtom.hpp"
#include "CMLAtomArray.hpp"
#include "CMLAtomParity.hpp"
#include "CMLBond.hpp"
#include "CMLBondArray.hpp"
#include "CMLBondStereo.hpp"
#include "CMLCml.hpp"
#include "CMLCrystal.hpp"
#include "CMLFormula.hpp"
#include "CMLLength.hpp"
#include "CMLMetadata.hpp"
#include "CMLMetadataList.hpp"
#include "CMLMolecule.hpp"
#include "CMLScalar.hpp"
#include "CMLSymmetry.hpp"
#include "CMLTorsion.hpp"

#include "CMLSaxHandler.hpp"
#include "CMLSerializer.hpp"

#ifdef USE_MPATROL
#include "mpatrol.hpp"
#endif

#ifdef _VC6
  #include "vc_mem_leak.hpp"

  #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    #define THIS_FILE __FILE__
  #endif  //#ifdef _DEBUG
#endif  // #ifdef _VC6


/* document class (== W3C DOM) */
using namespace std;
namespace CML {

    //! Constructor
    CMLDocument::CMLDocument() {
        init();
    }

    CMLDocument::CMLDocument(InputWrapper& input) : Document(input) {
        init();
    }

    CMLDocument::CMLDocument(const CMLDocument& d) :
        Document((Document) d)
        {
    }

    CMLDocument::~CMLDocument()
		{
		}

    void CMLDocument::init() {
    }

    Serializer *CMLDocument::getSerializerPtr() const {
        if (serializerPtr == 0) {
            serializerPtr = new CMLSerializer;
        }
        return serializerPtr;
    }

    bool CMLDocument::readXML(InputWrapper& is) {
        return readXML(is, debug);
    }

    bool CMLDocument::readXML(InputWrapper& input, bool debug) {
        Parser* parserPtr = new Parser;
        CMLSaxHandler* saxHandlerPtr = new CMLSaxHandler;
        saxHandlerPtr->setDebug (debug);
        saxHandlerPtr->setDocumentPtr (this);
        parserPtr->setSaxHandler (saxHandlerPtr);
        if (debug) {
            cerr << "using CML parser " << endl;
        }
        const bool b = parserPtr->parse(input);
        if (debug) {
            cerr << "parsed doc " << endl;
        }
        delete parserPtr;
        delete saxHandlerPtr;

        return b;
    }

		bool CMLDocument :: fixRootElements (Node *node_p)
			{
				Element *new_root_p = createCMLCml ();

				return mergeRootElements (new_root_p, node_p);
			}

/* factory methods */
    CMLAngle* CMLDocument::createCMLAngle() {
        return new CMLAngle(this);
    }

    CMLArray* CMLDocument::createCMLArray() {
        return new CMLArray(this);
    }

    CMLAtom* CMLDocument::createCMLAtom() {
        return new CMLAtom(this);
    }

    CMLAtomArray* CMLDocument::createCMLAtomArray() {
        return new CMLAtomArray(this);
    }

    CMLAtomParity* CMLDocument::createCMLAtomParity() {
        return new CMLAtomParity(this);
    }

    CMLBase* CMLDocument::createCMLBase(string tag) {
        return new CMLBase(this, tag);
    }

    CMLBond* CMLDocument::createCMLBond() {
        return new CMLBond(this);
    }

    CMLBondArray* CMLDocument::createCMLBondArray() {
        return new CMLBondArray(this);
    }

    CMLBondStereo* CMLDocument::createCMLBondStereo() {
        return new CMLBondStereo(this);
    }

    CMLCml* CMLDocument::createCMLCml() {
        return new CMLCml(this);
    }

    CMLCrystal* CMLDocument::createCMLCrystal() {
        return new CMLCrystal(this);
    }

    CMLFormula* CMLDocument::createCMLFormula() {
        return new CMLFormula(this);
    }

    CMLLength* CMLDocument::createCMLLength() {
        return new CMLLength(this);
    }

    CMLMetadata* CMLDocument::createCMLMetadata() {
        return new CMLMetadata(this);
    }

    CMLMetadataList* CMLDocument::createCMLMetadataList() {
        return new CMLMetadataList(this);
    }

    CMLMolecule* CMLDocument::createCMLMolecule() {
        return new CMLMolecule(this);
    }

    CMLScalar* CMLDocument::createCMLScalar() {
        return new CMLScalar(this);
    }

    CMLSymmetry* CMLDocument::createCMLSymmetry() {
        return new CMLSymmetry(this);
    }

    CMLTorsion* CMLDocument::createCMLTorsion() {
        return new CMLTorsion(this);
    }

    Text* CMLDocument::createText() {
        return new Text(this);
    }

    Text* CMLDocument::createText(const string &s) {
        Text* textPtr = new Text(this);
        textPtr->setNodeValue(s);
        return textPtr;
    }


} //end namespace CML


