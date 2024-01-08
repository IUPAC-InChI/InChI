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
#include "CMLTorsion.hpp"

using namespace std;
namespace CML {

    CMLTorsion::CMLTorsion() :
        CMLBase() {
        init();
    }

    CMLTorsion::CMLTorsion(CMLDocument* documentPtr) :
        CMLBase(documentPtr, C_TORSION) {
        init();
    }

    CMLTorsion::CMLTorsion(const CMLTorsion& t) :
        CMLBase((CMLBase) t),
        units(t.units),
        value(t.value)
        {
					atomRefs4Array [0] = t.atomRefs4Array [0];
					atomRefs4Array [1] = t.atomRefs4Array [1];
					atomRefs4Array [2] = t.atomRefs4Array [2];
					atomRefs4Array [3] = t.atomRefs4Array [3];

    }

    void CMLTorsion::init() {
				atomRefs4Array [0] =
				atomRefs4Array [1] =
				atomRefs4Array [2] =
				atomRefs4Array [3] =
				units = CH_EMPTY;

        value = 0.0;
    }

    CMLTorsion::~CMLTorsion() {
    }


}

