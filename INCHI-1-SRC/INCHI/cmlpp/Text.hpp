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


#ifndef TEXT_HPP
#define TEXT_HPP

#include <iostream>
#include <string>

#include "Node.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML
{

//! Text holds PCData

class CMLPP_API Text : public Node
{
public:
	Text (Document *documentPtr = 0);
	virtual ~Text ();

	virtual void writeXML (ostream &os);

	static const string TEXT;

private:
	void init ();
};

}

#endif //TEXT_HPP

