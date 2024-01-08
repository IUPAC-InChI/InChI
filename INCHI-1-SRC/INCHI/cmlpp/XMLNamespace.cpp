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
#include "XMLNamespace.hpp"

#ifdef USE_MPATROL
#include "mpatrol.hpp"
#endif

#ifdef _VC6
  #include "vc_mem_leak.hpp"

  #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    #define THIS_FILE __FILE__
  #endif  //#ifdef _DEBUG
#endif  // #ifdef _VC6

using namespace std;
namespace CML {


XMLNamespace::XMLNamespace (const string &prefix, const string &url)
	:	prefix (prefix),
		url (url)
{}

XMLNamespace::XMLNamespace (const XMLNamespace& s)
	: prefix (s.prefix),
		url (s.url)
{}


XMLNamespace::~XMLNamespace()
{}


XMLNamespace &XMLNamespace :: operator = (XMLNamespace &other)
{
	if (this != &other)
		{
			prefix = other.prefix;
			url = other.url;
		}

	return *this;
}


string XMLNamespace::getPrefixString() const
{
	return (prefix == CH_EMPTY) ? S_EMPTY : prefix + CH_COLON;
}


ostream &operator << (ostream &stm, const XMLNamespace &nspace_r)
{
	stm << "{prefix: ";
	stm << "prefix = " << nspace_r.prefix << "/";
	stm << "url = " << nspace_r.url << "}";

	return stm;
}


void XMLNamespace::setPrefix(const string& p)
{

	prefix = p;
}

const string &XMLNamespace::getPrefix() const
{

	return prefix;
}

void XMLNamespace::setURL(const string& u)
{

	url = u;
}

const string &XMLNamespace::getURL() const
{
	return url;
}

}	// namespace CML


