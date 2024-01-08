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


#ifndef CONVERSIONS_HPP
#define CONVERSIONS_HPP

#include <string>
#include "symbols.hpp"

using namespace std;

namespace CML
{

CMLPP_API char *fromDouble (const double &d);
CMLPP_API char *fromInt (const int &i);

CMLPP_API int tokenizeString (const string &str, const string & delim, char *** const tokens_pp);

}/*; extraneous semicolon commented out by DCh 1-18-2005 */

#endif	// CONVERSIONS_HPP
