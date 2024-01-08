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


#ifndef CML_ATOM_EDITOR_HPP
#define CML_ATOM_EDITOR_HPP

#include "CMLEditor.hpp"
#include "CMLAtom.hpp"
#include "hashtable.hpp"
#include "symbols.hpp"

namespace CML
{

class CMLPP_API CMLAtomEditor : public CMLEditor
{
public:
	static CMLAtomEditor *getEditor (CMLAtom * const atom_p);

	~CMLAtomEditor ();

	const CMLAtom * const getAtom ();

protected:
	CMLAtomEditor (CMLAtom * const atom_p);

private:
	static HashTable <CMLAtom *, CMLAtomEditor> ate_hashtable;

	CMLAtom *ate_atom_p;
};


}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML

#endif	/* CML_ATOM_EDITOR_HPP */
