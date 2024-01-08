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


#ifndef XMLUTILS_HPP
#define XMLUTILS_HPP

#include <iostream>
#include <string>
#include <vector>

#include "chars.hpp"
#include "xmlchars.hpp"
#include "XMLNamespace.hpp"
#include "symbols.hpp"


using namespace std;
namespace CML {

typedef pair <string,string> Attribute;
typedef vector <Attribute> AttributeVector;
typedef vector <string> StringVector;

/// A collection of XML processnig functions

class CMLPP_API XMLUtils {

public:
	XMLUtils ();

	virtual ~XMLUtils ();

	static string processXMLEntities (string &s);

	static void skippedEntityMessage (const string &name);

	static string escapeXMLEntities (const string& s);

	/// gets attribute of given name; 0 if not found
	static int getAttributeIndex (const AttributeVector &atts, const string& name);

	/// gets attribute of given name; CH_EMPTY if not found
	static string getAttributeValue (const AttributeVector &atts, const string& name);

	static string createAttributeString (const string& attName, const string& attValue);

	static void appendToArray (string &array, int value);

	static void appendToArray (string &array, double value);

	static void appendToArray (string &array, string &value);

	static void tokenizeStringToStringArray (StringVector &v, StringVector::size_type n, const string &att);

	static void tokenizeStringToIntArray (vector <int> &v, vector<int>::size_type n, const string &att);

	static void tokenizeStringToFloatArray (vector <double> &v, vector<double>::size_type n, const string &att);

	void printVector (StringVector& v, ostream& os);

	static void cmlError (const string msg);

	static void tokenize (StringVector &vcr, const string& buf, const string& delimstr);

	static void tokenize (StringVector &vcr, const string &s, const string& delimstr, int limit);

	// trims whitespace from start and end.
	static string &trim  (string &s);

	static void toLowerCase  (string &s);

	static void toUpperCase  (string &s);

	static bool isInStringVector (const StringVector &v, const string &s);

	//2010-02-07 static const Attribute EMPTYATT;
};

}



#if ( defined(WIN32) && defined(_MSC_VER) && _MSC_VER == 1200 )

#ifdef CMLPP_DLL_EXPORTS
#define DLLDECLSPEC __declspec(dllexport)
#else
#define DLLDECLSPEC /* __declspec(dllimport)*/
#endif

    extern "C" DLLDECLSPEC int __stdcall TestLoadDll( int n );

#endif

#endif // XMLUTILS_HPP

