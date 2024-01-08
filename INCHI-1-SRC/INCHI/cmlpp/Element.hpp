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


#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include <iostream>
#include <string>
#include <vector>

#include "XMLUtils.hpp"
#include "chars.hpp"
#include "xmlchars.hpp"
#include "cmlnames.hpp"

#include "XMLNamespace.hpp"
#include "Serializer.hpp"
#include "Node.hpp"
#include "SaxHandler.hpp"
#include "symbols.hpp"
#include "NodeList.hpp"

using namespace std;
namespace CML {


class Document;

/*base class for all elements */
class CMLPP_API Element : public Node {

public:
	Element ();
	Element (Document *documentPtr, string tagName);
	Element (const Element &e);
	Element &operator = (Element  &other);

	virtual ~Element ();

	virtual void startSAX (SaxHandler *saxPtr, AttributeVector &atts);
	virtual void endSAX (SaxHandler *saxPtr);

	virtual void writeXML (ostream &os) const;
	virtual void writeXML (ostream &os, const XMLNamespace *const namespacePtr) const;

	virtual int getAttributeIndex (const string &attName) const;

	virtual const string &getPCDATA () const;

	virtual const AttributeVector &getAttributes () const;
	virtual void setAttributes (AttributeVector &a);

	virtual string getAttributeValue (const string &attName) const;
	virtual void setAttributeValue (const string &attName, const string &attValue);
	virtual void setAttributeValue (const string &attName, int attValue);
	virtual void setAttributeValue (const string &attName, double attValue);

	virtual string getAttributeString (const string &attName) const;
	virtual void clearChildVector ();

	virtual NodeList *getElementsByTagName (const string &tag);

protected:
// attributes
	AttributeVector atts;

};

}

#endif //ELEMENT_HPP

