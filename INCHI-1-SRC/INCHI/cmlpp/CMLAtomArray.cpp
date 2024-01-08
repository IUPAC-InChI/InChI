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
#include "CMLAtomArray.hpp"

#include "CMLAtom.hpp"
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


CMLAtomArray::CMLAtomArray(CMLDocument* documentPtr)
	: CMLBase (documentPtr, C_ATOMARRAY),
		natoms (0)
{}



CMLAtomArray &CMLAtomArray :: operator = (CMLAtomArray &other)
{
	if (this != &other)
		{
			CMLAtom *atom_p;
			unsigned int i  = natoms;

			if (natoms > other.natoms)
				{
					for (i = 0; i < other.natoms; ++ i)
						{
							* (atomPtrVector [i]) = * (other.atomPtrVector [i]);
						}

					for (i = natoms - 1; i >= other.natoms; -- i)
						{
							atom_p = atomPtrVector [i];
							atomPtrVector.pop_back ();
							delete atom_p;
						}
				}
			else
				{
					for (i = 0; i < natoms; ++ i)
						{
							* (atomPtrVector [i]) = * (other.atomPtrVector [i]);
						}

					for (i = natoms; i < other.natoms; ++ i)
						{
							atom_p = new CMLAtom (* (other.atomPtrVector [i]));

							atomPtrVector.push_back (atom_p);
						}
				}

			natoms = other.natoms;

			idVector = other.idVector;
			elementTypeVector = other.elementTypeVector;
			formalChargeVector = other.formalChargeVector;
			hydrogenCountVector = other.hydrogenCountVector;
			x2Vector = other.x2Vector;
			y2Vector = other.y2Vector;
			x3Vector = other.x3Vector;
			y3Vector = other.y3Vector;
			z3Vector = other.z3Vector;
			xFractVector = other.xFractVector;
			yFractVector = other.yFractVector;
			zFractVector = other.zFractVector;
		}

	return *this;
}



CMLAtomArray::CMLAtomArray(const CMLAtomArray& other)
	: natoms (other.natoms),
    idVector (other.idVector),
    elementTypeVector (other.elementTypeVector),
    formalChargeVector (other.formalChargeVector),
    hydrogenCountVector (other.hydrogenCountVector),
    x2Vector (other.x2Vector),
    y2Vector (other.y2Vector),
    x3Vector (other.x3Vector),
    y3Vector (other.y3Vector),
    z3Vector (other.z3Vector),
    xFractVector (other.xFractVector),
    yFractVector (other.yFractVector),
    zFractVector (other.zFractVector)
{
	atomPtrVector.resize (natoms);
	CMLAtom *atom_p;

	for (unsigned int i = 0; i < natoms; ++ i)
		{
			atom_p = new CMLAtom (* (other.atomPtrVector [i]));
			atomPtrVector.push_back (atom_p);
		}
}


CMLAtomArray :: ~CMLAtomArray()
{}


void CMLAtomArray::startSAX(SaxHandler* saxPtr, AttributeVector& atts)
{
  CMLBase::startSAX(saxPtr, atts);
  CMLSaxHandler* cmlSaxPtr = dynamic_cast <CMLSaxHandler*> (saxPtr);

  if (cmlSaxPtr)
		{
			string atomID = XMLUtils::getAttributeValue(atts, C_ATOMID);
#ifdef CML_DEBUG
			cerr << "atomArray::atomID " << atomID << endl;
#endif
			if (atomID != CH_EMPTY)
				{
					cmlSaxPtr->setCmlType(C_CML2);
					cmlSaxPtr->setInputArray(true);
#ifdef CML_DEBUG
					cerr << "using CML2 and arrays " << endl;
#endif
				}
		}
	else
		{
#ifndef INCHI_LIB
      cerr << "program bug: CMLAtomArray::endSAX(SaxHandler* saxPtr)" << endl;
#endif
		}
}

    void CMLAtomArray::endSAX(SaxHandler* saxPtr) {
        StringVector sv;
        //CMLBase::endSAX(saxPtr);
        CMLSaxHandler* cmlSaxPtr = dynamic_cast <CMLSaxHandler*> (saxPtr);
        if (cmlSaxPtr)
       		{
						if (cmlSaxPtr->getInputArray()) {
								if (cmlSaxPtr->getCmlType() == C_CML2) {
#ifdef CML_DEBUG
										cerr << "document uses CML2 and arrays " << endl;
#endif
										string atomID = getAttributeValue(C_ATOMID);
										saxPtr->debugOutput("id:" + atomID + ":" +"\n");
										XMLUtils::tokenize(sv, atomID, CH_SPACE);
										natoms = sv.size();
										if (natoms == 0) {
												XMLUtils::cmlError("startAtomArray: No atoms given");
												return;
										}
										cmlSaxPtr->debugOutput(string("natoms:" + natoms) + ":");
										XMLUtils::tokenizeStringToStringArray(idVector, natoms, atomID);
										setAttributeValue(C_ATOMID, CH_EMPTY);
										XMLUtils::tokenizeStringToStringArray(elementTypeVector, natoms, getAttributeValue(C_ELEMENTTYPE));
										setAttributeValue(C_ELEMENTTYPE, CH_EMPTY);
										string fCharge = getAttributeValue(C_FORMALCHARGE);
										setAttributeValue(C_FORMALCHARGE, CH_EMPTY);
										XMLUtils::tokenizeStringToIntArray(formalChargeVector, natoms, fCharge);
										XMLUtils::tokenizeStringToIntArray(hydrogenCountVector, natoms, getAttributeValue(C_HYDROGENCOUNT));
										setAttributeValue(C_HYDROGENCOUNT, CH_EMPTY);
										XMLUtils::tokenizeStringToFloatArray(x2Vector, natoms, getAttributeValue(C_X2));
										setAttributeValue(C_X2, CH_EMPTY);
										XMLUtils::tokenizeStringToFloatArray(y2Vector, natoms, getAttributeValue(C_Y2));
										setAttributeValue(C_Y2, CH_EMPTY);
										XMLUtils::tokenizeStringToFloatArray(x3Vector, natoms, getAttributeValue(C_X3));
										setAttributeValue(C_X3, CH_EMPTY);
										XMLUtils::tokenizeStringToFloatArray(y3Vector, natoms, getAttributeValue(C_Y3));
										setAttributeValue(C_Y3, CH_EMPTY);
										XMLUtils::tokenizeStringToFloatArray(z3Vector, natoms, getAttributeValue(C_Z3));
										setAttributeValue(C_Z3, CH_EMPTY);
										XMLUtils::tokenizeStringToFloatArray(xFractVector, natoms, getAttributeValue(C_XFRACT));
										setAttributeValue(C_XFRACT, CH_EMPTY);
										XMLUtils::tokenizeStringToFloatArray(yFractVector, natoms, getAttributeValue(C_YFRACT));
										setAttributeValue(C_YFRACT, CH_EMPTY);
										XMLUtils::tokenizeStringToFloatArray(zFractVector, natoms, getAttributeValue(C_ZFRACT));
										setAttributeValue(C_ZFRACT, CH_EMPTY);
										if (CH_EMPTY != getAttributeValue(C_XYZ3) ||
										CH_EMPTY != getAttributeValue(C_XY2)) {
												XMLUtils::cmlError("attributes xyz3 and xy2 not supported in CML2 array mode");
										}
								} else {
#ifndef INCHI_LIB
										cerr << "document uses CML1 and arrays " << endl;
#endif
										natoms = 0;
		//                vector <Node*> cv = getChildVector();
										for (unsigned int i = 0; i < childVector.size(); i++) {
												string builtin;
												string array;
												convertBuiltinArrayToVector((CMLBase*)childVector[i], builtin, array);
												if (natoms == 0) {
														XMLUtils::tokenize(sv, array, CH_SPACE);
														natoms = sv.size();
														if (natoms == 0) {
																XMLUtils::cmlError("startAtomArray: No atoms given");
																return;
														}
												}
												if (false) {
												} else if (builtin == C_ID || builtin == C_ATOMID) {
														XMLUtils::tokenizeStringToStringArray(idVector, natoms, array);
												} else if (builtin == C_ELEMENTTYPE) {
														XMLUtils::tokenizeStringToStringArray(elementTypeVector, natoms, array);
												} else if (builtin == C_FORMALCHARGE) {
														XMLUtils::tokenizeStringToIntArray(formalChargeVector, natoms, array);
												} else if (builtin == C_HYDROGENCOUNT) {
														XMLUtils::tokenizeStringToIntArray(hydrogenCountVector, natoms, array);
												} else if (builtin == C_X2) {
														XMLUtils::tokenizeStringToFloatArray(x2Vector, natoms, array);
												} else if (builtin == C_Y2) {
														XMLUtils::tokenizeStringToFloatArray(y2Vector, natoms, array);
												} else if (builtin == C_X3) {
														XMLUtils::tokenizeStringToFloatArray(x3Vector, natoms, array);
												} else if (builtin == C_Y3) {
														XMLUtils::tokenizeStringToFloatArray(y3Vector, natoms, array);
												} else if (builtin == C_Z3) {
														XMLUtils::tokenizeStringToFloatArray(z3Vector, natoms, array);
												} else if (builtin == C_XFRACT) {
														XMLUtils::tokenizeStringToFloatArray(xFractVector, natoms, array);
												} else if (builtin == C_YFRACT) {
														XMLUtils::tokenizeStringToFloatArray(yFractVector, natoms, array);
												} else if (builtin == C_ZFRACT) {
														XMLUtils::tokenizeStringToFloatArray(zFractVector, natoms, array);
												} else if (builtin == C_XYZ3 || builtin == C_XY2) {
														XMLUtils::cmlError("attributes xyz3 and xy2 not supported in CML2 array mode");
												} else {
												}
										}
										clearChildVector();
								}
								if (documentPtr->getDebug()) {
										cerr << "creating " << natoms << " atoms" << endl;
								}
								CMLDocument* documentPtr = dynamic_cast <CMLDocument*>  (saxPtr->getDocumentPtr());
								if (documentPtr) {
									for (unsigned int i = 0; i < natoms; ++i) {
											CMLAtom* atomPtr = documentPtr->createCMLAtom();
											appendAtom (atomPtr);
											if (idVector.size() > 0) {
													atomPtr->setAttributeValue(C_ID, idVector[i]);
											}
											if (elementTypeVector.size() > 0) {
													atomPtr->setAttributeValue(C_ELEMENTTYPE, elementTypeVector[i]);
											}
											if (formalChargeVector.size() > 0) {
													atomPtr->setAttributeValue(C_FORMALCHARGE, formalChargeVector[i]);
											}
											if (x2Vector.size() > 0) atomPtr->setAttributeValue(C_X2, x2Vector[i]);
											if (y2Vector.size() > 0) atomPtr->setAttributeValue(C_Y2, y2Vector[i]);
											if (x3Vector.size() > 0) atomPtr->setAttributeValue(C_X3, x3Vector[i]);
											if (y3Vector.size() > 0) atomPtr->setAttributeValue(C_Y3, y3Vector[i]);
											if (z3Vector.size() > 0) atomPtr->setAttributeValue(C_Z3, z3Vector[i]);
											if (xFractVector.size() > 0) atomPtr->setAttributeValue(C_XFRACT, xFractVector[i]);
											if (yFractVector.size() > 0) atomPtr->setAttributeValue(C_YFRACT, yFractVector[i]);
											if (zFractVector.size() > 0) atomPtr->setAttributeValue(C_ZFRACT, zFractVector[i]);
											atomPtr->setUntypedAttributes();
									}
								}
									else
								{
#ifndef INCHI_LIB
										cerr << "program bug: CMLAtomArray::endSAX" << endl;
#endif
								}


						} else {
#ifdef CML_DEBUG
								cerr << "document uses cmlType " << cmlSaxPtr->getCmlType() << " and array " << cmlSaxPtr->getInputArray() << endl;
#endif
								atomPtrVector.clear();

								natoms = childVector.size ();

								for (unsigned int i = 0; i < natoms; ++ i) {
										atomPtrVector.push_back (static_cast <CMLAtom *> (childVector[i]));
								}


						}


					}
				else

       	{
#ifndef INCHI_LIB
            cerr << "program bug: CMLAtomArray::endSAX(SaxHandler* saxPtr)" << endl;
#endif
        }
    }

    void CMLAtomArray::writeXML(ostream &os) const {
        const CMLSerializer* const cmlSerializerPtr = dynamic_cast <const CMLSerializer* const> (getDocumentPtr()->getSerializerPtr());
        if (!cmlSerializerPtr) {
#ifndef INCHI_LIB
            cerr << "program bug: CMLAtomArray::writeXML" << endl;
#endif
        }
        if (documentPtr->getDebug()) {
            cerr << endl << "<!--output AtomArray: " << cmlSerializerPtr->getType();
            cerr << "serializer " << *cmlSerializerPtr << "-->" << endl;
        }
        string cmlType = cmlSerializerPtr->getCmlType();
        bool array = cmlSerializerPtr->getArray();
        if (documentPtr->getDebug()) {
            cerr << "cmlType: " << cmlType << "; array: " << array << endl;
        }
        cmlSerializerPtr->writeStartTagStart(os, C_ATOMARRAY);
        if (array) {

// array
// CML2
            vector <CMLBase*> basePtrVector;
            for (unsigned int i = 0; i < atomPtrVector.size(); i++) {
                basePtrVector.push_back(atomPtrVector[i]);
            }
            if (cmlType == C_CML2) {
                outputCML2Attribute(os, C_ATOMID, basePtrVector);
                outputCML2Attribute(os, C_ELEMENTTYPE, basePtrVector);
                outputCML2Attribute(os, C_FORMALCHARGE, basePtrVector);

                outputCML2Attribute(os, C_X2, basePtrVector);
                outputCML2Attribute(os, C_Y2, basePtrVector);

                outputCML2Attribute(os, C_XFRACT, basePtrVector);
                outputCML2Attribute(os, C_YFRACT, basePtrVector);
                outputCML2Attribute(os, C_ZFRACT, basePtrVector);

                outputCML2Attribute(os, C_X3, basePtrVector);
                outputCML2Attribute(os, C_Y3, basePtrVector);
                outputCML2Attribute(os, C_Z3, basePtrVector);

                cmlSerializerPtr->writeStartTagEnd(os);
                os << endl;

            } else {
// CML1
                cmlSerializerPtr->writeStartTagEnd(os);
                os << endl;
                outputCML1Array(os, C_STRINGARRAY, C_ATOMID, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_STRINGARRAY, C_ELEMENTTYPE, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_INTEGERARRAY, C_FORMALCHARGE, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_FLOATARRAY, C_X2, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_FLOATARRAY, C_Y2, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_FLOATARRAY, C_X3, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_FLOATARRAY, C_Y3, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_FLOATARRAY, C_Z3, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_FLOATARRAY, C_XFRACT, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_FLOATARRAY, C_YFRACT, cmlSerializerPtr, basePtrVector);
                outputCML1Array(os, C_FLOATARRAY, C_ZFRACT, cmlSerializerPtr, basePtrVector);
            }
        } else {
// non-array
            cmlSerializerPtr->writeStartTagEnd(os);
            for (unsigned int i = 0; i < atomPtrVector.size(); i++) {
                CMLAtom* atomPtr = atomPtrVector[i];
                atomPtr->writeXML(os);
            }
        }
        cmlSerializerPtr->writeEndTag(os, C_ATOMARRAY);
    }


void CMLAtomArray :: appendAtom (CMLAtom * const atom_p)
{
	atomPtrVector.push_back (atom_p);
	appendChild (atom_p);
}

}	// namespace CML

