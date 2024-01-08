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


#ifndef DOCUMENT_HPP
#define DOCUMENT_HPP

#include <string>
#include <vector>

#include "Node.hpp"
#include "InputWrapper.hpp"

#include "symbols.hpp"

/* document class (== W3C DOM) */
using namespace std;
namespace CML {

	class Element;

class CMLPP_API Document : public Node
{


public:
	//! Constructor
	Document();
	Document(InputWrapper &is);
	Document(const Document& doc);
	Document &operator = (Document &other);

	virtual ~Document();

	virtual bool readXML(InputWrapper& input);
	virtual bool readXML(InputWrapper& input, bool debug);

	virtual void writeXML(ostream &os);

	virtual void appendChild(Node* nodePtr);

	virtual void setSerializerPtr(Serializer* sPtr);
	virtual Serializer *getSerializerPtr() const;

	void setRootElementPtr(Element* rep);
	Element* getRootElementPtr() const;

	virtual void setDebug(bool d);
	virtual bool getDebug() const;


protected:
	virtual bool fixRootElements (Node *node_p);


	bool mergeRootElements (Element *new_root_p, Node *node_p);


	mutable Serializer *serializerPtr;
	bool debug;

	// root element
	Element* rootElementPtr;

private:
	void init();

};

} //end namespace CML

#endif //DOCUMENT_HPP
