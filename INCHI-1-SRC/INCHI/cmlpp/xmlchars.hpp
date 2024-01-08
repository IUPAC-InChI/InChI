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


#ifndef XMLCHARS_H
#define XMLCHARS_H

#include <string>

using namespace std;
namespace CML
{

// XML strings
static const string X_AMP              = "amp";
static const string X_AMP_HASH         = "&#";
static const string X_APOS             = "apos";
static const string X_DOCTYPE          = "<!DOCTYPE";
static const string X_ENCODING         = "encoding";
static const string X_LT               = "lt";
static const string X_GT               = "gt";
static const string X_NO               = "no";
static const string X_QUOT             = "quot";
static const string X_SPACE_NEWLINE    = " \n";
static const string X_STANDALONE       = "standalone";
static const string X_SYSTEM           = "SYSTEM";
static const string X_VERSION          = "version";
static const string X_VERSION_NUMBER   = "1.0";
static const string X_XMLDECL          = "<?xml";
static const string X_XMLNS            = "xmlns";
static const string X_XML              = "xml";
static const string E_TAGO             = "</";
static const string E_PI               = "?>";
static const string S_PI               = "<?";
static const string S_COMMENT          = "<!--";
static const string E_COMMENT          = "-->";
static const string S_CDATA            = "<![CDATA[";
static const string E_CDATA            = "]]>";
}

#endif // XMLCHARS_H

