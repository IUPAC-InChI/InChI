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
#include "CMLAtomEditor.hpp"

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
HashTable <CMLAtom *, CMLAtomEditor> CMLAtomEditor :: ate_hashtable;


CMLAtomEditor *CMLAtomEditor :: getEditor (CMLAtom * const atom_p)
{
	CMLAtomEditor *editor_p = const_cast <CMLAtomEditor *> (ate_hashtable.getValue (atom_p));

	if (!editor_p)
		{
			editor_p = new CMLAtomEditor (atom_p);
			ate_hashtable.put (atom_p, editor_p);
		}

	return editor_p;
}


CMLAtomEditor :: CMLAtomEditor (CMLAtom * const atom_p)
	: ate_atom_p (atom_p)
{}


CMLAtomEditor :: ~CMLAtomEditor ()
{
	ate_hashtable.remValue (ate_atom_p);
}


const CMLAtom * const CMLAtomEditor :: getAtom ()
{
	return ate_atom_p;
}




}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML

