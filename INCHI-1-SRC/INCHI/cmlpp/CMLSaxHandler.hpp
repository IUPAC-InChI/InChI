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


#ifndef CMLSAX_HPP
#define CMLSAX_HPP

#include <string>
#include <iostream>
#include <vector>

#include "chars.hpp"
#include "cmlnames.hpp"
#include "XMLUtils.hpp"

#include "Parser.hpp"
#include "Node.hpp"
#include "SaxHandler.hpp"
#include "CMLDocument.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLSaxHandler : public SaxHandler {

public:
	CMLSaxHandler();
	CMLSaxHandler(const CMLSaxHandler& sh);
	CMLSaxHandler &operator = (CMLSaxHandler & other);

	virtual ~CMLSaxHandler();

	virtual bool startDocument();

	virtual bool endDocument();

	virtual void startElement(const string &namespaceURI, const string &localName, const string &prefix, AttributeVector &atts);

	virtual void endElement(const string &namespaceURI, const string &localName, const string &prefix);

	void setCmlType(const string& c);
	const string &getCmlType() const;

	void setInputArray(bool b);
	bool getInputArray() const;

	void setUseBuiltin(bool b);
	bool getUseBuiltin() const;

	vector <CMLBase*>& getElementPtrVector();
	void pushElement(CMLBase* elemPtr);
	CMLBase* popElement();
	CMLBase* peekElement();

	virtual const string &getType() const;

protected:
	vector <CMLBase*> elementPtrVector;
	// using builtins?
	bool useBuiltin;

	string cmlType;
	// are we using array syntax for input
	bool inputArray;

	Node* parentNodePtr;

private:
	CMLDocument *cmlDocumentPtr;
	void init();

};

}

#endif // CMLSAX_HPP

