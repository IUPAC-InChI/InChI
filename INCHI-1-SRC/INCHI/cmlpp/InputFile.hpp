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


#ifndef INPUT_FILE_HPP
#define INPUT_FILE_HPP

#include <cstdio>
#include "symbols.hpp"

#include "InputWrapper.hpp"


namespace CML
{

class CMLPP_API InputFile : public InputWrapper
{
private:
	FILE *if_file_p;

public:
	InputFile (FILE *file_p);
	~InputFile ();

	virtual bool getLine (string *buffer_p);
};


}	// namespace CML


#endif	// INPUT_FILE_HPP
