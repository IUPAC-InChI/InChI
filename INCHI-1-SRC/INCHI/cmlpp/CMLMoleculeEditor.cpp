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
#include "CMLMoleculeEditor.hpp"

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

namespace CML
{

// initialise statics
HashTable <CMLMolecule *, CMLMoleculeEditor> CMLMoleculeEditor :: moe_hashtable;


CMLMoleculeEditor *CMLMoleculeEditor :: getEditor (CMLMolecule * const molecule_p)
{
	CMLMoleculeEditor *editor_p = const_cast <CMLMoleculeEditor *> (moe_hashtable.getValue (molecule_p));

	if (!editor_p)
		{
			editor_p = new CMLMoleculeEditor (molecule_p);
			moe_hashtable.put (molecule_p, editor_p);
		}

	return editor_p;
}


CMLMoleculeEditor :: CMLMoleculeEditor (CMLMolecule * const molecule_p)
	: moe_molecule_p (molecule_p)
{}


CMLMoleculeEditor :: ~CMLMoleculeEditor ()
{
	moe_hashtable.remValue (moe_molecule_p);
}


const CMLMolecule * const CMLMoleculeEditor :: getMolecule ()
{
	return moe_molecule_p;
}


CMLAtomArray *CMLMoleculeEditor :: getAtomArray ()
{
	return dynamic_cast <CMLAtomArray *> (moe_molecule_p -> getFirstChildWithName (C_ATOMARRAY));
}


CMLBondArray *CMLMoleculeEditor :: getBondArray ()
{
	return dynamic_cast <CMLBondArray *> (moe_molecule_p -> getFirstChildWithName (C_BONDARRAY));
}



}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML

