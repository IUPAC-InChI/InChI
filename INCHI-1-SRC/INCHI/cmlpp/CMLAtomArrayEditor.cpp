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
#include "CMLAtomArrayEditor.hpp"


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
HashTable <CMLAtomArray *, CMLAtomArrayEditor> CMLAtomArrayEditor :: aae_hashtable;


CMLAtomArrayEditor *CMLAtomArrayEditor :: getEditor (CMLAtomArray * const atom_array_p)
{
	CMLAtomArrayEditor *editor_p = const_cast <CMLAtomArrayEditor *> (aae_hashtable.getValue (atom_array_p));

	if (!editor_p)
		{
			editor_p = new CMLAtomArrayEditor (atom_array_p);
			aae_hashtable.put (atom_array_p, editor_p);
		}

	return editor_p;
}


CMLAtomArrayEditor :: CMLAtomArrayEditor (CMLAtomArray * const atom_array_p)
	: aae_atom_array_p (atom_array_p)
{}


CMLAtomArrayEditor :: ~CMLAtomArrayEditor ()
{
	aae_hashtable.remValue (aae_atom_array_p);
}



unsigned int CMLAtomArrayEditor :: getNumberOfAtoms () const
{
	return aae_atom_array_p -> natoms;
}


const CMLAtom * const CMLAtomArrayEditor :: getAtom (const unsigned int index) const
{
	if (index < (aae_atom_array_p -> atomPtrVector.size ()))
		{
			return (aae_atom_array_p -> atomPtrVector [index]);
		}
	else
		{
			return 0;
		}
}


void CMLAtomArrayEditor :: appendAtom (CMLAtom * const atom_p)
{
	aae_atom_array_p -> appendAtom (atom_p);
}


}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML

