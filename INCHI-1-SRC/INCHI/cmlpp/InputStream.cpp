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
#include "InputStream.hpp"

namespace CML
{

InputStream :: InputStream (istream &is)
	: is_stream_p (&is)
{}


InputStream :: ~InputStream ()
{}


bool InputStream :: getLine (string *buffer_p)
{
	return !!getline (*is_stream_p, *buffer_p);  // added !! to return bool -- 2004-06-19 DTch */
}

}	// namespace CML
