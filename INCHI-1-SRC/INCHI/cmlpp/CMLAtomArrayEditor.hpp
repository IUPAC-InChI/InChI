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


#ifndef CML_ATOM_ARRAY_EDITOR_HPP
#define CML_ATOM_ARRAY_EDITOR_HPP

#include "CMLEditor.hpp"
#include "CMLAtomArray.hpp"
#include "hashtable.hpp"
#include "symbols.hpp"

namespace CML
{

/// This allows programmatic altering of a CMLAtomArray

class CMLPP_API CMLAtomArrayEditor : public CMLEditor
{
public:
	/// Returns a pointer to a CMLAtomArrayEditor for the given
	/// CMLAtomArray pointed to atom_array_p.
	static CMLAtomArrayEditor *getEditor (CMLAtomArray * const atom_array_p);

	/// returns the number of atoms in the editor's CMLAtomArray
	unsigned int getNumberOfAtoms () const;

	/// returns a constant pointer to a constant CMLAtom at the given
	/// index in the editor's CMLAtomArray. returns 0 if the index is
	/// out of range.
	const CMLAtom * const getAtom (const unsigned int index) const;

	/// Adds the CMLAtom atom_p to both the editor's CMLAtomArray and

	/// to the DOM of the underlying CMLDocument.
	void appendAtom (CMLAtom * const atom_p);

	~CMLAtomArrayEditor ();

protected:
	CMLAtomArrayEditor (CMLAtomArray * const atom_p);

private:
	static HashTable <CMLAtomArray *, CMLAtomArrayEditor> aae_hashtable;

	CMLAtomArray *aae_atom_array_p;
};


}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML

#endif	/* CML_ATOM_ARRAY_EDITOR_HPP */
