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
#include <cstdio>


#include "Element.hpp"
#include "Document.hpp"

#include "Text.hpp"

#include <deque>

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
namespace CML
{


Element :: Element ()
{}


Element :: Element (Document *dPtr, string tagName)
	: Node (dPtr, tagName)
{}


Element :: Element (const Element &e)
	: atts (e.atts)
{}


Element &Element  ::  operator = (Element &other)
{
	if (this != &other)
		{
			atts = other.atts;
		}

	return *this;
}


Element :: ~Element ()
{}


void Element :: startSAX (SaxHandler *sPtr, AttributeVector &atts)
{
	setAttributes(atts);
}

// all subclasses are no-ops unless overridden
void Element :: endSAX (SaxHandler *sPtr)
{
	// deliberate no-op
}

void Element :: writeXML (ostream &os) const
{
	writeXML(os, 0);
}


void Element :: writeXML (ostream &os, const XMLNamespace * const namespacePtr) const
{
	unsigned int i = atts.size ();
	const Attribute *att_p = & (atts [0]);

	Serializer *serializerPtr = getSerializerPtr ();


	serializerPtr -> writeStartTagStart (os, nodeName);
	for ( ; i > 0; -- i, ++ att_p) {
			os << XMLUtils :: createAttributeString (att_p -> first, att_p -> second);
	}
	if (namespacePtr !=  0) {
			os << XMLUtils :: createAttributeString(X_XMLNS+CH_COLON+namespacePtr->getPrefix(), namespacePtr->getURL());
	}
	serializerPtr -> writeStartTagEnd(os);

	string s = getPCDATA ();
	if (s.length () > 0)
		{
			os << s;
		}

	const Node **node_pp = const_cast <const Node **> (& (childVector [0]));

	for (i = childVector.size (); i > 0; -- i, ++ node_pp) {
			(*node_pp) -> writeXML (os);
	}
	serializerPtr -> writeEndTag (os, nodeName);

}

const string &Element :: getPCDATA() const {
	 static string pcd = CH_EMPTY;
	 vector <Node*> v = this->getChildVector();
	 if (v.size() == 1) {
			 if (v[0]->getNodeName() == Text :: TEXT) {
					 pcd = v[0]->getNodeValue();
			 }
	 }
	 return pcd;
}

int Element :: getAttributeIndex(const string& attName) const {
		return XMLUtils :: getAttributeIndex(atts, attName);
}

string Element :: getAttributeValue(const string& attName) const {
		string value = CH_EMPTY;
		int idx = getAttributeIndex(attName);
		if (idx >= 0) {
				value = atts[idx].second;
		}
		return value;
}

string Element :: getAttributeString(const string& attName) const {
	 return XMLUtils :: createAttributeString(attName, getAttributeValue(attName));
}

void Element :: setAttributeValue(const string& attName, int attValue) {
		setAttributeValue(attName, ""+attValue);
}


void Element :: setAttributeValue(const string& attName, double attValue) {
		char* temp = new char[80];
		sprintf(temp, "%.4f", attValue);
		string s(temp);
		setAttributeValue(attName, s);
}


void Element :: setAttributeValue(const string &attName, const string &attValue) {
		int idx = getAttributeIndex(attName);
		if (idx == -1) {
				Attribute newAtt (attName, attValue);

				atts.push_back(newAtt);
		} else {
				atts[idx].second = attValue;
		}
}

const AttributeVector& Element :: getAttributes() const
{
	return atts;
}

void Element :: setAttributes(AttributeVector& a)
{
	atts = a;
}

void Element :: clearChildVector()
{
	Node :: clearChildVector();
}


NodeList *Element :: getElementsByTagName (const string &tag)
{
	deque <Node *> nodes_to_consider;
	vector <Node *> nodes_found;

	Node *node_p;
	Node **node_pp;
	unsigned int num_nodes;
	NodeList *node_list_p = 0;

	num_nodes = childVector.size ();

	if (num_nodes)
		{
			node_pp = & (childVector [0]);

			for ( ; num_nodes > 0; -- num_nodes, ++ node_pp)
				{
					nodes_to_consider.push_back (*node_pp);
				}


			while (!nodes_to_consider.empty ())
				{
					node_p = nodes_to_consider.front ();

					nodes_to_consider.pop_front ();

					// add the children of node_p
					vector <Node *> children = node_p -> getChildVector ();

					if ((num_nodes = children.size ()) > 0)
						{
							node_pp = & (children [0]);

							for ( ; num_nodes > 0; -- num_nodes, ++ node_pp)
								{
									nodes_to_consider.push_back (*node_pp);
								}
						}

					// do we have a match?
					//cerr << "node " << node_p -> getNodeName () << " tag " << tag << endl;
					if (node_p -> getNodeName () == tag)
						{
							nodes_found.push_back (node_p);
						}

				}

			if ((num_nodes = nodes_found.size ()) > 0)
				{
					node_list_p = new NodeList (nodes_found);

					return node_list_p;
				}
		}

	// no children
	return 0;
}


}	// namespace CML
