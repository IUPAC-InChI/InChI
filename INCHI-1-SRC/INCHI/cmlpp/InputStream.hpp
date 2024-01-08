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


#ifndef INPUT_STREAM_HPP
#define INPUT_STREAM_HPP

#include <iostream>
#include <string>
#include "symbols.hpp"

#include "InputWrapper.hpp"

using namespace std;

namespace CML
{

class CMLPP_API InputStream : public InputWrapper
{
private:
	istream *is_stream_p;

public:
	InputStream (istream &is);
	~InputStream ();

	virtual bool getLine (string *buffer_p);





};





}



#endif 	// INPUT_STREAM_HPP
