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


#ifndef PARSER_HPP
#define PARSER_HPP

#include "XMLUtils.hpp"
#include "SaxHandler.hpp"
#include "Document.hpp"
#include "symbols.hpp"

#include "InputWrapper.hpp"

using namespace std;
namespace CML {


class CMLPP_API Parser {

public:
	Parser ();

	Parser  (const Parser &other);
	Parser &operator = (Parser &other);
	virtual ~Parser ();

	void setSaxHandler (SaxHandler *sh);

	bool parse (InputWrapper &is);

	void setDebug (bool d) {debug = d;}

	// utility routines

	static bool isXMLName (const string &n);

	static void splitAttributes (string &s, AttributeVector &atts);

	// normalizes string
	static string getNormalizedString (const string &s);

	// normalizes string
	static string getNormalizedString (const char *ch);


	static void writePCDATA (ostream&os, const string &value);

protected:
	bool debug;

private:
	SaxHandler *saxHandlerPtr;
	bool inComment;

	void init ();

	// process anything in balanced <...>
	void tag  (string &s);

	string startTag (const string &s);

};

}

#endif // PARSER_HPP

