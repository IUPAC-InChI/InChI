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
#include "CMLBondArrayEditor.hpp"

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
HashTable <CMLBondArray *, CMLBondArrayEditor> CMLBondArrayEditor :: bae_hashtable;


CMLBondArrayEditor *CMLBondArrayEditor :: getEditor (CMLBondArray * const bond_array_p)
{
	CMLBondArrayEditor *editor_p = const_cast <CMLBondArrayEditor *> (bae_hashtable.getValue (bond_array_p));

	if (!editor_p)
		{
			editor_p = new CMLBondArrayEditor (bond_array_p);
			bae_hashtable.put (bond_array_p, editor_p);
		}

	return editor_p;
}


CMLBondArrayEditor :: CMLBondArrayEditor (CMLBondArray * const bond_array_p)
	: bae_bond_array_p (bond_array_p)
{}


CMLBondArrayEditor :: ~CMLBondArrayEditor ()
{
	bae_hashtable.remValue (bae_bond_array_p);
}


unsigned int CMLBondArrayEditor :: getNumberOfBonds () const
{
	return bae_bond_array_p -> nbonds;
}


const CMLBond * const CMLBondArrayEditor :: getBond (const unsigned int index) const
{
	if (index < (bae_bond_array_p -> bondPtrVector.size ()))
		{
			return (bae_bond_array_p -> bondPtrVector [index]);
		}
	else
		{
			return 0;
		}
}


void CMLBondArrayEditor :: appendBond (CMLBond * const bond_p)
{
	bae_bond_array_p -> appendBond (bond_p);
}


}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML

