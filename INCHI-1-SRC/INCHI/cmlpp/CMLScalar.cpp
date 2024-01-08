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
#include "CMLScalar.hpp"
#include "CMLSaxHandler.hpp"

using namespace std;
namespace CML {

    CMLScalar::CMLScalar() :
        CMLBase() {
        init();
    }

    CMLScalar::CMLScalar(CMLDocument* documentPtr) :
        CMLBase(documentPtr, C_SCALAR) {
        init();
    }

    void CMLScalar::init() {
        units = CH_EMPTY;
        dataType = CH_EMPTY;
    }

    CMLScalar::~CMLScalar() {
    }

    void CMLScalar::endSAX(SaxHandler* saxPtr) {
        CMLBase::endSAX(saxPtr);
        dataType = getAttributeValue(C_DATATYPE);
        units = getAttributeValue(C_UNITS);
    }

}

