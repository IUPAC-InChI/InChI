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

#include <cstdlib> // Added 04-25-2010 (to work with gcc 4.3/4.4; thanks to Peter Linstrom)

#include "pragma.hpp"
#include "CMLCrystal.hpp"
#include "CMLSaxHandler.hpp"
#include "CMLSymmetry.hpp"

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

    CMLCrystal::CMLCrystal() :
        CMLBase() {
        init();
    }

    CMLCrystal::CMLCrystal(CMLDocument* documentPtr)  :
        CMLBase(documentPtr, C_CRYSTAL) {
        init();
    }

    CMLCrystal::CMLCrystal(const CMLCrystal& c) :
        CMLBase((CMLBase) c),

        spacegroup(c.spacegroup),
        pointgroup(c.pointgroup)
        {

				symmetryPtr = new CMLSymmetry (*c.symmetryPtr);
    }

    CMLCrystal::~CMLCrystal() {
			delete symmetryPtr;
    }

    void CMLCrystal::init() {
        symmetryPtr = 0;
        spacegroup = CH_EMPTY;
        pointgroup = CH_EMPTY;
    }

    void CMLCrystal::setCellParam(const unsigned int num, double value) {
        cellParam[num] = value;
    }

    void CMLCrystal::setCellParam(const unsigned int num, const string& value) {
        if (num >= 0 && num <= 5) {
            double v = atof((char*)value.c_str());
            setCellParam(num, v);
        }
    }


	double CMLCrystal::getCellParam (const unsigned int num) const
	{
		return cellParam[num];
    }

}

