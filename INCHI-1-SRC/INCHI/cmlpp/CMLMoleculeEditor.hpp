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


#ifndef CML_MOLECULE_EDITOR_HPP
#define CML_MOLECULE_EDITOR_HPP

#include "CMLEditor.hpp"
#include "CMLMolecule.hpp"
#include "hashtable.hpp"
#include "symbols.hpp"

namespace CML
{

class CMLPP_API CMLMoleculeEditor : public CMLEditor
{
public:
	static CMLMoleculeEditor *getEditor (CMLMolecule * const molecule_p);

	~CMLMoleculeEditor ();

	const CMLMolecule * const getMolecule ();
	CMLAtomArray *getAtomArray ();
	CMLBondArray *getBondArray ();

protected:
	CMLMoleculeEditor (CMLMolecule * const molecule_p);

private:
	static HashTable <CMLMolecule *, CMLMoleculeEditor> moe_hashtable;

	CMLMolecule *moe_molecule_p;
};


}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML

#endif	/* CML_MOLECULE_EDITOR_HPP */
