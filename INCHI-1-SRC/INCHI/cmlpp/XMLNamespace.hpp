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


#ifndef NAMESPACE_HPP
#define NAMESPACE_HPP

#include <iostream>
#include <string>

#include "xmlchars.hpp"
#include "XMLUtils.hpp"

#include "cmlnames.hpp"
#include "symbols.hpp"


using namespace std;
namespace CML {

class CMLPP_API XMLNamespace
{
  friend ostream &operator << (ostream &stm, const XMLNamespace &nspace_r);

public:
	//! Constructor
//	XMLNamespace ();
	XMLNamespace (const string &prefix = CH_EMPTY, const string &url = CH_EMPTY);

	XMLNamespace (const XMLNamespace &s);
	XMLNamespace &operator = (XMLNamespace &other);

	//! Destructor
	virtual ~XMLNamespace ();

	void setPrefix (const string &p);
	const string &getPrefix () const;

	void setURL (const string &u);
	const string &getURL () const;

	string getPrefixString() const;

private:
	string prefix;
	string url;

	void init ();
};

} //end namespace

#endif //NAMESPACE_HPP
