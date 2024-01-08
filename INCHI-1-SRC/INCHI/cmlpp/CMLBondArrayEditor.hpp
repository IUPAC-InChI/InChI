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


#ifndef CML_BOND_ARRAY_EDITOR_HPP
#define CML_BOND_ARRAY_EDITOR_HPP

#include "CMLEditor.hpp"
#include "CMLBondArray.hpp"
#include "hashtable.hpp"
#include "symbols.hpp"

namespace CML
{

/**	An editor allowing a CMLBondArray to be progammed.
	*
	*	This gives an API to access the internals of CMLBondArray
	* rather than via the DOM.
	*/
class CMLPP_API CMLBondArrayEditor : public CMLEditor
{
public:
	/** This static method is the way to create an editor */
	static CMLBondArrayEditor *getEditor (CMLBondArray * const bond_array_p);

	unsigned int getNumberOfBonds () const;	/**< Returns the number of bonds in the array */

	/** Returns a constant pointer to a constant bond for the given index,
		* or 0 if the index is out of range.
		*/

	const CMLBond * const getBond (const unsigned int index) const;


	/** Adds the bond to both the DOM and the BondArray */
	void appendBond (CMLBond * const bond_p);

	/** Destructor */
	~CMLBondArrayEditor ();

protected:
	CMLBondArrayEditor (CMLBondArray * const bond_p);

private:
	static HashTable <CMLBondArray *, CMLBondArrayEditor> bae_hashtable;

	CMLBondArray *bae_bond_array_p;
};


}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML

#endif	/* CML_BOND_ARRAY_EDITOR_HPP */
