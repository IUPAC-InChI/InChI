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
#include "Node.hpp"

#include "Document.hpp"
#include "Element.hpp"

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

    Node* Node::currentNodePtr = 0;

    Node::Node()
    	:	parentNodePtr (0),
        documentPtr (0),
        nodeName (CH_EMPTY),
        nodeValue (CH_EMPTY)
    	{}

    Node::Node(Document* dPtr, string name)
    	:	parentNodePtr (0),
        documentPtr (dPtr),
        nodeName (name),
        nodeValue (CH_EMPTY)
    {}

    Node::Node(const Node& node) :
        parentNodePtr(node.parentNodePtr),
        documentPtr(node.documentPtr),
        nodeName(node.nodeName),
        nodeValue(node.nodeValue)
    {
			const unsigned int numChildren = node.childVector.size ();

			if (numChildren)
				{
					childVector.resize (numChildren);
					for (unsigned int i = 0; i < numChildren; ++ i)
						{
							* (childVector [i]) = * (node.childVector [i]);
						}
				}
    }

		Node &Node :: operator = (Node &other)
			{
				if (this != &other)
					{
						parentNodePtr = other.parentNodePtr;
						documentPtr = other.documentPtr;
						nodeName = other.nodeName;
						nodeValue = other.nodeValue;

						const unsigned int this_children = childVector.size ();
						const unsigned int other_children = other.childVector.size ();
						Node *node_p;
						unsigned int i = 0;

						if (this_children > other_children)
							{
								for ( ; i < other_children; ++ i)
									{
										* (childVector [i]) = * (other.childVector [i]);
									}

								for (i = this_children - 1; i >= other_children; -- i)
									{
										node_p = childVector [i];
										childVector.pop_back ();
										delete node_p;
									}
							}
						else
							{
								for ( ; i < this_children; ++ i)
									{
										* (childVector [i]) = * (other.childVector [i]);
									}

								for (i = this_children; i < other_children; ++ i)
									{
										node_p = new Node (* (other.childVector [i]));

										childVector.push_back (node_p);
									}
							}
					}

				return *this;
			}


    Node::~Node() {
			int i = (static_cast <int> (childVector.size ())) - 1;
			Node *ptr;

			for ( ; i >= 0; -- i) {
				ptr = childVector [i];
				childVector.pop_back ();

				//cerr << "freeing " << ptr -> nodeName << "_:_" << ptr -> nodeValue  << endl;
				delete ptr;
			}
    }

    Document* Node::getDocumentPtr() const {
        return documentPtr;
    }

    void Node::appendChild(Node* nodePtr) {
        if (documentPtr->getRootElementPtr() == 0) {
            documentPtr->setRootElementPtr((Element*)nodePtr);
            currentNodePtr = documentPtr->getRootElementPtr();
        } else {
            nodePtr->setParentNodePtr(currentNodePtr);
        }
        childVector.push_back(nodePtr);
    }

    void Node::writeXML(ostream &os) const {
        // always overwritten
    }

    Node* Node::getFirstChildWithName(const string& name) {
        vector <Node*> nameVector = getChildrenWithName(name);
        return (nameVector.size() > 0) ? nameVector[0] : 0;
    }

    vector <Node*> Node::getChildrenWithName(const string& name) {
        vector <Node*> nameVector;
        for (unsigned int i = 0; i < childVector.size(); ++i) {
            if (childVector[i]->getNodeName() == name) {
                nameVector.push_back(childVector[i]);
            }
        }
        return nameVector;
    }

// return current serializer; if zero get one from document
    Serializer *Node::getSerializerPtr() const {
				if (documentPtr)
					{
						return documentPtr->getSerializerPtr();
					}
				else
					{
						return 0;
					}
    }


     const vector <Node*> & Node::getChildVector() const {return childVector;}
     void Node::clearChildVector() { childVector.clear();}


     Node* Node::getParentNodePtr() const {return parentNodePtr;}
     void Node::setParentNodePtr(Node* nodePtr) {parentNodePtr = nodePtr;}

     void Node::setDocumentPtr(Document* d) {documentPtr = d;}


     const string& Node::getNodeName() const
		 {
			return nodeName;
		}

		 void Node::setNodeName(const string& s)
		 {
		 nodeName = s;
		 }

     const string& Node::getNodeValue() const {
		 return nodeValue;
		 }

		 void Node::setNodeValue(const string& s) {
		 nodeValue = s;
		 }
}
