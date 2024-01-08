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


#ifndef SAX_HPP
#define SAX_HPP

#include <string>
#include <iostream>
#include <vector>

#include "XMLUtils.hpp"
#include "chars.hpp"
#include "cmlnames.hpp"

#include "symbols.hpp"
#include "Document.hpp"

using namespace std;
namespace CML {


typedef vector<XMLNamespace> XMLNamespaceVector;

class CMLPP_API SaxHandler {

public:
	static XMLNamespaceVector &getXMLNamespacePtrVector ();

	SaxHandler ();
	SaxHandler (const SaxHandler &s);
	SaxHandler &operator =  (SaxHandler &other);
	virtual ~SaxHandler ();

	virtual void setDebug (bool d) {debug = d;}
	virtual bool getDebug () {return debug;}
	virtual void debugOutput  (const string &s) const;
	virtual void debugOutput  (const char * const text_p) const;

	// SAX-like call back
	virtual bool startDocument ();
	virtual bool endDocument ();

	virtual void startElement (const string &name, AttributeVector &atts);

	Attribute getXMLNamespacePtrPair (string &name);

	virtual void startElement (const string &namespaceURI, const string &localName, const string &prefix, AttributeVector &atts);

	virtual void processAttributes (AttributeVector &atts);

	virtual void processXMLNamespace (const string &name, const string &value);

	virtual void endElement (const string &name);

	virtual void endElement (const string &namespaceURI, const string &localName, const string &prefix);

	virtual void characters (string &s);

	virtual void processingInstruction (const string &target, const string &data);

	virtual XMLNamespace *getXMLNamespacePtr () const;
	virtual void setXMLNamespacePtr (XMLNamespace *s);

	virtual const AttributeVector &getCurrentAtts () const;
	virtual void setCurrentAtts (const AttributeVector &av);

	virtual const string &getPCDATA () const;
	virtual void setPCDATA (const string &s);

	virtual Document *getDocumentPtr () const;
	virtual void setDocumentPtr (Document *d);

	virtual string getCurrentElementName () const;
	void setCurrentElementName (const string &en);

	virtual const string &getParent () const;
	void setParent (const string &en);

	const StringVector &getElementNameStack () const;

	virtual const string &getType () const;

protected:
	Document *documentPtr;

	XMLNamespace *nspacePtr;
	bool ownsNspacePtr;

	StringVector elementNameStack;
	// current parent element name
	string currentElementName;
	// current element name
	string parent;
	// last PCDATA  (text) content read
	string pcdata;
	// current attributes
	AttributeVector currentAtts;
	// namespaces
	bool debug;

	static XMLNamespaceVector namespaceVector;

private:
	void init ();
};

}

#endif // SAX_HPP

