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


#ifndef CMLNODE_HPP
#define CMLNODE_HPP


#include <iostream>
#include <string>
#include <vector>


#include "Serializer.hpp"
#include "symbols.hpp"

using namespace std;
namespace CML {

class Document;

/*node class for all XMLNodes */

class CMLPP_API Node {

public:
	Node ();
	Node (Document *documentPtr, string name);
	Node (const Node &node);
	Node &operator = (Node &other);
	virtual ~Node ();

	//    virtual Node *const getChild (int i) const {return childVector.get (i);}
	virtual const vector <Node *>  &getChildVector () const;
	virtual void clearChildVector ();

	virtual void appendChild (Node *nodePtr);

	virtual Node *getParentNodePtr () const;
	virtual void setParentNodePtr (Node *nodePtr);

	virtual Document *getDocumentPtr () const;
	virtual void setDocumentPtr (Document *d);

	virtual Serializer *getSerializerPtr () const;
//	virtual void setSerializerPtr (Serializer *s);

	virtual const string &getNodeName () const;
	virtual void setNodeName (const string &s);

	virtual const string &getNodeValue () const;
	virtual void setNodeValue (const string &s);

	virtual void writeXML (ostream &os) const;

	Node *getFirstChildWithName (const string &name);
	vector <Node *> getChildrenWithName (const string &name);

protected:
	Node *parentNodePtr;
	// children
	vector <Node *> childVector;

	// owner document
	Document *documentPtr;
	// w3 node name
	string nodeName;
	// w3 node value
	string nodeValue;

private:
	static Node *currentNodePtr;
	void init ();


};


}

#endif //CMLNODE_HPP

