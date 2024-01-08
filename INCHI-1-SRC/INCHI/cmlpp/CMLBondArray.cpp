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
#include "CMLBondArray.hpp"

#include "CMLBond.hpp"
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

using namespace std;
namespace CML {

    CMLBondArray::CMLBondArray() :
        CMLBase() {
        init();
    }

    CMLBondArray::CMLBondArray(const CMLBondArray& other) :
        nbonds (other.nbonds),
        idVector (other.idVector),
        atomRef1Vector (other.atomRef1Vector),
        atomRef2Vector (other.atomRef2Vector),
        orderVector (other.orderVector),
        stereoVector (other.stereoVector)
        {
						bondPtrVector.resize (nbonds);
					CMLBond *bond_p;

					for (unsigned int i = 0; i < nbonds; ++ i)
						{
							bond_p = new CMLBond (* (other.bondPtrVector [i]));
							bondPtrVector.push_back (bond_p);
						}

    }


	CMLBondArray &CMLBondArray :: operator = (CMLBondArray &other)
		{
			if (this != &other)
				{
					idVector = other.idVector;
					atomRef1Vector = other.atomRef1Vector;
					atomRef2Vector = other.atomRef2Vector;
					orderVector = other.orderVector;
					stereoVector = other.stereoVector;

					CMLBond *bond_p;
					unsigned int i  = nbonds;

					if (nbonds > other.nbonds)
						{
							for (i = 0; i < other.nbonds; ++ i)
								{
									* (bondPtrVector [i]) = * (other.bondPtrVector [i]);
								}

								for (i = nbonds - 1; i >= other.nbonds; -- i)
									{
										bond_p = bondPtrVector [i];
										bondPtrVector.pop_back ();
										delete bond_p;
									}
						}
					else
						{
							for (i = 0; i < nbonds; ++ i)
								{
									* (bondPtrVector [i]) = * (other.bondPtrVector [i]);
								}

							for (i = nbonds; i < other.nbonds; ++ i)
								{
									bond_p = new CMLBond (* (other.bondPtrVector [i]));

									bondPtrVector.push_back (bond_p);
								}
						}


					nbonds = other.nbonds;
				}


			return *this;
		}

    void CMLBondArray::init() {
        bondPtrVector.clear();
        nbonds = 0;
        idVector.clear();
        atomRef1Vector.clear();
        atomRef2Vector.clear();
        orderVector.clear();
        stereoVector.clear();
    }

    CMLBondArray::CMLBondArray(CMLDocument* documentPtr)  : CMLBase(documentPtr, C_BONDARRAY) {
    }

    CMLBondArray::~CMLBondArray() {
/*
			int i = (static_cast <int> (bondPtrVector.size ())) - 1;
			CMLBond *ptr;

			for ( ; i >= 0; -- i) {
				ptr = bondPtrVector [i];
				bondPtrVector.pop_back ();
				delete ptr;
				ptr = 0;
			}
*/
    }

    void CMLBondArray::endSAX(SaxHandler* saxPtr) {
        StringVector sv;
        //CMLBase::endSAX(saxPtr);
        CMLSaxHandler* cmlSaxPtr = dynamic_cast <CMLSaxHandler*> (saxPtr);
        if (!cmlSaxPtr) {
#ifndef INCHI_LIB
            cerr << "program bug: CMLBondArray::endSAX(SaxHandler* saxPtr)" << endl;
#endif
        }

        string atomRef1 = XMLUtils::getAttributeValue(atts, C_ATOMREF1);
        if (atomRef1 != CH_EMPTY) {
            cmlSaxPtr->setCmlType(C_CML2);
            cmlSaxPtr->setInputArray(true);
        }
        if (cmlSaxPtr->getInputArray()) {
            if (cmlSaxPtr->getCmlType() == C_CML2) {
                string atomRef1 = getAttributeValue(C_ATOMREF1);
                cmlSaxPtr->debugOutput("id:" + atomRef1 + ":");
                XMLUtils::tokenize(sv, atomRef1, CH_SPACE);
                nbonds = sv.size();
                if (nbonds == 0) {
                    XMLUtils::cmlError("startBondArray: No bonds given");
                    return;
                }
                cmlSaxPtr->debugOutput(string("nbonds:" + nbonds) + ":   ");
                XMLUtils::tokenizeStringToStringArray(atomRef1Vector, nbonds, getAttributeValue(C_ATOMREF1));
                setAttributeValue(C_ATOMREF1, CH_EMPTY);
                XMLUtils::tokenizeStringToStringArray(atomRef2Vector, nbonds, getAttributeValue(C_ATOMREF2));
                setAttributeValue(C_ATOMREF2, CH_EMPTY);
                XMLUtils::tokenizeStringToStringArray(orderVector, nbonds, getAttributeValue(C_ORDER));
                setAttributeValue(C_ORDER, CH_EMPTY);
                XMLUtils::tokenizeStringToStringArray(stereoVector, nbonds, getAttributeValue(C_STEREO));
                setAttributeValue(C_STEREO, CH_EMPTY);
            } else {
                nbonds = 0;
//                vector <Node*> cv = getChildVector();
                for (unsigned int i = 0; i < childVector.size(); i++) {
                    string builtin;
                    string array;
                    convertBuiltinArrayToVector((CMLBase*)childVector[i], builtin, array);
                    if (nbonds == 0) {
                        XMLUtils::tokenize(sv, array, CH_SPACE);
                        nbonds = sv.size();
                        if (nbonds == 0) {
                            XMLUtils::cmlError("startBondArray: No bonds given");
                            return;
                        }
                    }
                    if (false) {
                    } else if (builtin == C_ID || builtin == C_BONDID) {
                        XMLUtils::tokenizeStringToStringArray(idVector, nbonds, array);
                    } else if (builtin == C_ATOMREF) {
                        if (atomRef1Vector.size() == 0) {
                            XMLUtils::tokenizeStringToStringArray(atomRef1Vector, nbonds, array);
                        } else {
                            XMLUtils::tokenizeStringToStringArray(atomRef2Vector, nbonds, array);
                        }
                    } else if (builtin == C_ORDER) {
                        XMLUtils::tokenizeStringToStringArray(orderVector, nbonds, array);
                    } else if (builtin == C_STEREO) {
                        XMLUtils::tokenizeStringToStringArray(stereoVector, nbonds, array);
                    } else {
                    }
                }
                clearChildVector();
            }
            CMLDocument* documentPtr = dynamic_cast <CMLDocument*> (saxPtr->getDocumentPtr());
            if (!documentPtr) {
#ifndef INCHI_LIB
                cerr << "cast bug at: CMLBondArray::endSAX" << endl;
#endif
            }
            for (unsigned int i = 0; i < nbonds; ++i) {
                CMLBond* bondPtr = documentPtr->createCMLBond();
								appendBond (bondPtr);

                if (idVector.size() > 0) {
                    bondPtr->setAttributeValue(C_BONDID, idVector[i]);
                }
                if (atomRef1Vector.size() > 0 && atomRef2Vector.size()) {
                    bondPtr->setAttributeValue(C_ATOMREFS2, atomRef1Vector[i]+" "+ atomRef2Vector[i]);
                }
                if (orderVector.size() > 0) {
                    bondPtr->setAttributeValue(C_ORDER, orderVector[i]);
                }
                if (stereoVector.size() > 0) {
                    bondPtr->setAttributeValue(C_STEREO, stereoVector[i]);
                }
                bondPtr->setUntypedAttributes();
            }
        } else {
						nbonds = childVector.size ();
            for (unsigned int i = 0; i < nbonds; ++ i) {
                bondPtrVector.push_back((CMLBond*)childVector[i]);
            }


        }
    }

    bool CMLBondArray::getAtomRefs2Array(const vector <CMLBond*>& bondPtrVector, pair <string, string> **atom_refs2_pp) const {
        string atomRef1;
        string atomRef2;
        for (unsigned int i = 0; i < bondPtrVector.size(); i++) {
            CMLBond* bondPtr = bondPtrVector[i];
            if (i != 0) {
                atomRef1 += " ";
                atomRef2 += " ";
            }
            string atomRefs2S = bondPtr->getAttributeValue(C_ATOMREFS2);
            bondPtr->setAtomRefs2(atomRefs2S);
            atomRef1 += bondPtr->getAtomRefs2().first;
            atomRef2 += bondPtr->getAtomRefs2().second;
        }

				if (!atom_refs2_pp)
					{
						*atom_refs2_pp = new pair <string, string> (atomRef1, atomRef2);
					}
				else
					{
						(*atom_refs2_pp) -> first = atomRef1;
						(*atom_refs2_pp) -> second = atomRef1;
					}
				return true;
    }


    void CMLBondArray::writeXML(ostream &os) const {
        const CMLSerializer* const cmlSerializerPtr = dynamic_cast <const CMLSerializer* const > (getDocumentPtr()->getSerializerPtr());
        if (!cmlSerializerPtr) {
#ifndef INCHI_LIB
            cerr << "program bug: CMLBondArray::writeXML" << endl;
#endif
        }
        if (getDocumentPtr()->getDebug()) {
            cerr << endl << "<!--output AtomArray: " << cmlSerializerPtr->getType();
            cerr << "serializer " << *cmlSerializerPtr << "-->" << endl;
        }
        string cmlType = cmlSerializerPtr->getCmlType();
        bool array = cmlSerializerPtr->getArray();
        cmlSerializerPtr->writeStartTagStart(os, C_BONDARRAY);
#ifdef CML_DEBUG
        cerr << "writing bonds" << endl;
#endif
        if (array) {
// array
// CML2
            vector <CMLBase*> basePtrVector;
            for (unsigned int i = 0; i < bondPtrVector.size(); i++) {
                basePtrVector.push_back(bondPtrVector[i]);
            }

          	pair <string, string> *atom_refs2_array_p = new pair <string, string>;

            if (cmlType == C_CML2) {
                outputCML2Attribute(os, C_BONDID, basePtrVector);
                getAtomRefs2Array(bondPtrVector, &atom_refs2_array_p);
                os << XMLUtils::createAttributeString(C_ATOMREF1, atom_refs2_array_p -> first);
                os << XMLUtils::createAttributeString(C_ATOMREF2, atom_refs2_array_p -> second);
                outputCML2Attribute(os, C_ORDER, basePtrVector);
                cmlSerializerPtr->writeStartTagEnd(os);
                os << endl;
            } else {
// CML1
                cmlSerializerPtr->writeStartTagEnd(os);
                os << endl;
                outputCML1Array(os, C_STRINGARRAY, C_BONDID, cmlSerializerPtr, basePtrVector);
                getAtomRefs2Array(bondPtrVector, &atom_refs2_array_p);
                outputCML1Builtin(os, C_STRINGARRAY, C_ATOMREF, atom_refs2_array_p -> first, cmlSerializerPtr);
                outputCML1Builtin(os, C_STRINGARRAY, C_ATOMREF, atom_refs2_array_p -> second, cmlSerializerPtr);
                outputCML1Array(os, C_STRINGARRAY, C_ORDER, cmlSerializerPtr, basePtrVector);
            }
        } else {
// non-array
            cmlSerializerPtr->writeStartTagEnd(os);
            for (unsigned int i = 0; i < bondPtrVector.size(); i++) {
                CMLBond* bondPtr = bondPtrVector[i];
                bondPtr->writeXML(os);
            }
        }
        cmlSerializerPtr->writeEndTag(os, C_BONDARRAY);
    }


void CMLBondArray :: appendBond (CMLBond * const bond_p)
{
	bondPtrVector.push_back (bond_p);
	appendChild (bond_p);
}

}

