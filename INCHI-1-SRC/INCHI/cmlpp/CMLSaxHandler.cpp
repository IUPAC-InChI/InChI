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
#include "CMLSaxHandler.hpp"

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
#include "CMLMolecule.hpp"
#include "CMLScalar.hpp"
#include "CMLTorsion.hpp"

using namespace std;
namespace CML {
    const string &CMLSaxHandler :: getType() const
			{
				static string type = "CML-SAX";

				return type;
			}


    CMLSaxHandler::CMLSaxHandler() :
        SaxHandler(),
        cmlType(CH_EMPTY) {
        init();
    }

    void CMLSaxHandler::init() {
        useBuiltin = false;
        cmlType = CH_EMPTY;
        inputArray = false;
    }

    CMLSaxHandler::CMLSaxHandler(const CMLSaxHandler& sh) :
        SaxHandler((SaxHandler) sh),
        useBuiltin(sh.useBuiltin),
        cmlType(sh.cmlType),
        inputArray(sh.inputArray)
        {
    }

    CMLSaxHandler::~CMLSaxHandler() {
    }


// ------------------------ SAX events -------------------
    // SAX-like call back
    bool CMLSaxHandler::startDocument() {
    // clear all internals
        SaxHandler::startDocument();
        setUseBuiltin(false);
        setInputArray(false);
        setCmlType(CH_EMPTY);
        cmlDocumentPtr = dynamic_cast <CMLDocument*> (documentPtr);

        if (cmlDocumentPtr)
						{
			        parentNodePtr = cmlDocumentPtr;
			        return true;
						}
				else
					{
#ifndef INCHI_LIB
            cerr << "must use a CMLDocument with CMLSaxHandler" << endl;
#endif
            return false;
					}
    }

    // SAX-like call back
    bool CMLSaxHandler::endDocument() {
        bool b = SaxHandler::endDocument();
        debugOutput("endDoc");

        return b;
    }


// main start routine
    void CMLSaxHandler::startElement(const string &namespaceURI, const string &localName, const string &prefix, AttributeVector &atts) {

        SaxHandler::startElement(namespaceURI, localName, prefix, atts);
        //CMLBase* basePtr;
        CMLBase* basePtr = 0; // 11-17-2005 DT
        if (debug) { cerr << "<!--CML start-->" << endl;}
        useBuiltin = false;
        if (localName == C_ATOM) {
            basePtr = cmlDocumentPtr->createCMLAtom();
        } else if (localName == C_ATOMARRAY) {
            basePtr = cmlDocumentPtr->createCMLAtomArray();
        } else if (localName == C_ATOMPARITY) {
            setCmlType(C_CML2);
            basePtr = cmlDocumentPtr->createCMLAtomParity();
        } else if (localName == C_BOND) {
            basePtr = cmlDocumentPtr->createCMLBond();
        } else if (localName == C_BONDARRAY) {
            basePtr = cmlDocumentPtr->createCMLBondArray();
        } else if (localName == C_BONDSTEREO) {
            basePtr = cmlDocumentPtr->createCMLBondStereo();
        } else if (localName == C_CML) {
            basePtr = cmlDocumentPtr->createCMLCml();
        } else if (localName == C_CRYSTAL) {
            basePtr = cmlDocumentPtr->createCMLCrystal();
        } else if (localName == C_ELECTRON) {
//						basePtr = cmlDocumentPtr->createCMLElectron();
        } else if (localName == C_FORMULA) {
            basePtr = cmlDocumentPtr->createCMLFormula();
        } else if (localName == C_MOLECULE) {
            debugOutput("<!-- making Molecule -->");
            basePtr = cmlDocumentPtr->createCMLMolecule();
        } else if (
            localName == C_COORDINATE2   ||
            localName == C_COORDINATE3   ||
            localName == C_FLOAT         ||
            localName == C_INTEGER       ||
            localName == C_STRING) {
            basePtr = cmlDocumentPtr->createCMLBase(localName);
            setCmlType(C_CML1);
        } else if (
            localName == C_FLOATMATRIX   ||
            localName == C_FLOATARRAY    ||
            localName == C_INTEGERARRAY  ||
            localName == C_STRINGARRAY) {
            setCmlType(C_CML1);
            setInputArray(true);
            basePtr = cmlDocumentPtr->createCMLBase(localName);
        } else if (localName == C_LENGTH) {
            setCmlType(C_CML2);
            basePtr = cmlDocumentPtr->createCMLLength();
        } else if (localName == C_ANGLE) {
// only compiles if split... ???
            CMLAngle* anglePtr = cmlDocumentPtr->createCMLAngle();
            basePtr = (CMLBase*) anglePtr;
        } else if (localName == C_TORSION) {
            basePtr = cmlDocumentPtr->createCMLTorsion();
        } else if (localName == C_SCALAR) {
            basePtr = cmlDocumentPtr->createCMLScalar();
            setCmlType(C_CML2);
        } else if (localName == C_ARRAY) {
            basePtr = cmlDocumentPtr->createCMLArray();
            setCmlType(C_CML2);
        } else if (localName == C_MATRIX) {
            basePtr = cmlDocumentPtr->createCMLBase(localName);
            setCmlType(C_CML2);
    // other CML2 elements
        } else if (
            localName == "substance"     ||
            localName == "substanceList" ||
            localName == "amount") {
            basePtr = cmlDocumentPtr->createCMLBase(localName);
            setCmlType(C_CML2);
    // other STMML elements
        } else if (
            localName == "actionList"    ||
            localName == "action"        ||
            localName == "alternative"   ||
            localName == "annotation"    ||
            localName == "appinfo"       ||
            localName == "definition"    ||
            localName == "description"   ||
            localName == "dictionary"    ||
            localName == "dimension"     ||
            localName == "documentation" ||
            localName == "entry"         ||
            localName == "enumeration"   ||
            localName == "list"          ||
            localName == "link"          ||
            localName == "metadata"      ||
            localName == "metadataList"  ||
            localName == "object"        ||
            localName == "observation"   ||
            localName == "relatedEntry"  ||
            localName == "table"         ||
            localName == "unit"          ||
            localName == "unitType"      ||
            localName == "unitList") {
            basePtr = cmlDocumentPtr->createCMLBase(localName);
            debugOutput("<!--other " + localName + "-->");
            setCmlType(C_CML2);
        } else {
            basePtr = cmlDocumentPtr->createCMLBase(localName);
            if (debug) {
      		    XMLUtils::cmlError("unknown start element " + localName);
            }
        }
        if (debug) { cerr << "made:" << basePtr <<  endl;}
        if (debug) { cerr << "<!-- type:" << getCmlType() <<  " -->" << endl;}
        if (basePtr != 0) {

            parentNodePtr->appendChild(basePtr);
            pushElement(basePtr);
            if (debug) { cerr << "startSAX for " << basePtr <<  endl;}
            basePtr->startSAX(this, atts);
            if (debug) { cerr << "end startSAX" <<  endl;}
            parentNodePtr = basePtr;
        }
        debugOutput("<!-- CMLSAXendStart -->");
    }

    void CMLSaxHandler::endElement(const string &namespaceURI, const string &localName, const string &prefix) {
        string pcd = getPCDATA ();

				XMLUtils :: trim (pcd);

        SaxHandler::endElement(namespaceURI, localName, prefix);
        StringVector strings;

        string name (localName);
				XMLUtils :: trim (name);

        CMLBase* elementPtr = popElement();
        if (pcd != CH_EMPTY) {
            debugOutput("<!-- added text:" + pcd + ": -->");
            Text* textPtr = cmlDocumentPtr->createText(pcd);

            elementPtr->appendChild(textPtr);
        }
        parentNodePtr = peekElement();

        if (!parentNodePtr)
        	{
#ifdef CML_DEBUG
						cerr << "setting parentNodePtr to doc" << endl;
#endif
						parentNodePtr = cmlDocumentPtr;
					}
        string parent = getParent();
        if ( elementPtr )  // 11-17-2005 DT
        elementPtr->endSAX(this);
    }

    CMLBase* CMLSaxHandler::popElement() {
        CMLBase* elemPtr = peekElement ();
        if (elemPtr)
        	{
		        elementPtrVector.pop_back ();
		        return elemPtr;
					}
        else
        	{
						return 0;
					}
    }

    CMLBase* CMLSaxHandler::peekElement() {
        return ((elementPtrVector.size () > 0) ? elementPtrVector.back () : 0);
    }

    void CMLSaxHandler::pushElement(CMLBase* elemPtr) {
        elementPtrVector.push_back(elemPtr);
    }

    // records whether this is CMLV1.0 or CML2 (Schema)
    void CMLSaxHandler::setCmlType(const string& ct) {
        if (cmlType == CH_EMPTY) {
            if (debug) {
                cerr << "CML type ascertained as; " << ct << endl;
            }
            cmlType = ct;
        } else if (cmlType != ct) {
              XMLUtils::cmlError("Cannot mix CML types, was: " + cmlType);
        }
    }

const string &CMLSaxHandler::getCmlType() const
{
	return cmlType;
}

void CMLSaxHandler::setInputArray(bool b)
{
	inputArray = b;
}

bool CMLSaxHandler::getInputArray() const
{
	return inputArray;
}

void CMLSaxHandler::setUseBuiltin(bool b)
{
	useBuiltin = b;
}

bool CMLSaxHandler::getUseBuiltin() const
{
	return useBuiltin;
}

vector <CMLBase*>& CMLSaxHandler::getElementPtrVector()
{
	return elementPtrVector;
}


}


