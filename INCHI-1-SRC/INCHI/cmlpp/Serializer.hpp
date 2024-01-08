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


#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <iostream>
#include <string>
#include <vector>

#include "chars.hpp"
#include "xmlchars.hpp"
#include "XMLUtils.hpp"



// should really subclass this class
#include "cmlnames.hpp"
#include "symbols.hpp"

#include "XMLNamespace.hpp"

using namespace std;
namespace CML {

class Document;

/// This class is reponsible for writing a Document to a stream

class CMLPP_API Serializer {

	friend ostream &operator << (ostream &stm, const Serializer &ser_r);


public:
	//! Constructor
	Serializer ();
	Serializer (const string &doctype, bool useDeclaration, bool usePretty, XMLNamespace *nspacePtr, bool debug);

	Serializer (const Serializer &s);
	Serializer &operator =  (Serializer &other);
	//! Destructor
	virtual ~Serializer ();

	virtual string getType () const;

	/* *outputs XML header
	*
	 *outputs declaration, doctype if requested
	*/
	virtual void outputHeader (ostream &os) const;

	virtual void writeAttribute (ostream &os, const string &name, int value) const;

	virtual void writeAttribute (ostream &os, const string &name, double value) const;

	virtual void writeAttribute (ostream &os, const string &name, const string &value) const;

	virtual void writeStartTagStart (ostream &os, const string &name) const;

	virtual void writeStartTagEnd (ostream &os) const;

	virtual void writeEndTag (ostream &os, const string &name) const;

	virtual void writeCombinedTagEnd (ostream &os) const;

	virtual void setDoctype (const string &d);
	virtual const string &getDoctype () const;

	virtual void setDeclaration (bool b);
	virtual bool getDeclaration () const;

	virtual void setPretty (bool b);
	virtual bool getPretty () const;

	virtual void setXMLNamespacePtr (XMLNamespace * const n);
	virtual const XMLNamespace *getXMLNamespacePtr () const;

	virtual void setDebug (bool b);
	virtual bool getDebug () const;

protected:
	static Serializer *defaultSerializerPtr;

	string doctype;
	bool useDeclaration;
	bool pretty;
	/*const*/ XMLNamespace *nspacePtr;
	bool debug;

private:
	void init ();

};

} //end namespace

#endif //SERIALIZER_HPP
