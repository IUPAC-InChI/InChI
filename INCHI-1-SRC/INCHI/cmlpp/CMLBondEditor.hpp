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


#ifndef CML_BOND_EDITOR_HPP
#define CML_BOND_EDITOR_HPP

#include "CMLEditor.hpp"
#include "CMLBond.hpp"
#include "hashtable.hpp"
#include "symbols.hpp"

namespace CML
{

class CMLPP_API CMLBondEditor : public CMLEditor
{
public:
	static CMLBondEditor *getEditor (CMLBond * const bond_p);

	~CMLBondEditor ();

	const CMLBond * const getBond ();

protected:
	CMLBondEditor (CMLBond * const bond_p);

private:
	static HashTable <CMLBond *, CMLBondEditor> boe_hashtable;

	CMLBond *boe_bond_p;
};


}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML

#endif	/* CML_BOND_EDITOR_HPP */
