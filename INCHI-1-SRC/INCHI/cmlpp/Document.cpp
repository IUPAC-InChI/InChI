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
#include "Document.hpp"
#include "Element.hpp"
#include "Parser.hpp"
#include "SaxHandler.hpp"
#include "XMLUtils.hpp"
#include "Text.hpp"

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

/* document class (== W3C DOM) */
using namespace std;
namespace CML {
   //! Constructor
    Document::Document()
			:	serializerPtr (0),
        debug (false),
        rootElementPtr (0)
					{
    }

    Document::Document(InputWrapper& input)
			:	serializerPtr (0),
        debug (false),
        rootElementPtr (0)
			{
        readXML(input);
    }

    Document::Document(const Document& doc)
    :  debug (doc.debug),
        rootElementPtr(doc.rootElementPtr)

        {
				if (doc.serializerPtr)
					{
						serializerPtr = new Serializer (* (doc.serializerPtr));
					}
				else
					{
						serializerPtr = 0;
					}
    }

		Document &Document :: operator = (Document &other)
			{
				if (this != &other)
					{
						if (rootElementPtr)
							{
								if (other.rootElementPtr)
									{
										*rootElementPtr = * (other.rootElementPtr);
									}
								else
									{
										delete rootElementPtr;
										rootElementPtr = 0;
									}
							}
						else
							{
								if (other.rootElementPtr)
									{
										rootElementPtr = new Element (*other.rootElementPtr);
									}
							}

						if (serializerPtr)
							{
								if (other.serializerPtr)
									{
										*serializerPtr = * (other.serializerPtr);
									}
								else
									{
										delete serializerPtr;
										serializerPtr = 0;
									}
							}
						else
							{
								if (other.serializerPtr)
									{
										serializerPtr = new Serializer (* (other.serializerPtr));
									}
							}

					}

				return *this;
			}


    Document::~Document() {
        delete serializerPtr;
			}

    void Document::setDebug(bool d) {
        getSerializerPtr();
        (const_cast <Serializer * const> (serializerPtr))->setDebug(d);
        debug = d;
    }

    Serializer *Document::getSerializerPtr() const{
        if (serializerPtr == 0) {
            serializerPtr = new Serializer;
        }
        return serializerPtr;
    }

    void Document::appendChild(Node* nodePtr) {
			if ((nodeName == Text::TEXT) || ((nodeName.length () > 0) && (nodeName.substr(0,1) == "#")))
				{
					Node::appendChild(nodePtr);
					childVector.push_back(nodePtr);
				}
			else
				{
					if (rootElementPtr == 0)
						{
							rootElementPtr = (Element*)nodePtr;
							childVector.push_back(nodePtr);
						}
					else
						{
							fixRootElements (nodePtr);
							// XMLUtils::cmlError ("Fixed attempt to add more than one root element to doc, proceeding ");
						}
				}

    }

		bool Document :: fixRootElements (Node *node_p)
			{
				Element *new_root_p = new Element ();
				new_root_p -> setDocumentPtr (this);

				return mergeRootElements (new_root_p, node_p);
			}

		bool Document :: mergeRootElements (Element *new_root_p, Node *node_p)
			{
				Element *old_root_p = rootElementPtr;

				rootElementPtr = new_root_p;

				new_root_p -> appendChild (old_root_p);
				new_root_p -> appendChild (node_p);

				// remove old root from child vector and replace it with new one
				bool replaced_root = false;
				const unsigned int num_children = childVector.size ();
				for (unsigned int i = 0; i < num_children; ++ i)
					{
						if (childVector [i] == old_root_p)
							{
								childVector [i] = new_root_p;
								replaced_root = true;
							}

					}

				if (!replaced_root)
					{
						childVector.push_back (new_root_p);
					}

				return true;
			}


    bool Document::readXML(InputWrapper& input) {
        return readXML(input, getDebug());
    }

    bool Document::readXML(InputWrapper& input, bool debug) {
        Parser* parserPtr = new Parser;
        SaxHandler* saxHandlerPtr = new SaxHandler;
        saxHandlerPtr->setDebug(debug);
        saxHandlerPtr->setDocumentPtr(this);
        parserPtr->setSaxHandler(saxHandlerPtr);
        const bool b = parserPtr->parse(input);
        delete parserPtr;
        delete saxHandlerPtr;

        return b;
    }

    void Document::writeXML(ostream &os) {
        serializerPtr = getSerializerPtr();
        if (serializerPtr != 0) {
            os << "<!-- serializer: " << serializerPtr->getType() << "-->" << endl;
        } else {
            os << "<!--No serializer for doc-->" << endl;
        }
        if (rootElementPtr == 0) {
            os << "<!--DOCUMENT has no rootElement-->" << endl;
        } else {
            serializerPtr->outputHeader(os);
            rootElementPtr->writeXML(os, serializerPtr->getXMLNamespacePtr());
        }
    }

	void Document::setSerializerPtr(Serializer* sPtr) {
		serializerPtr = sPtr;
	}

	void Document::setRootElementPtr(Element* rep)
	{
		rootElementPtr = rep;
	}

	Element* Document::getRootElementPtr() const
	{
		return rootElementPtr;
	}

	bool Document::getDebug() const {return debug;}




} //end namespace


