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
#include "SaxHandler.hpp"
#include "Parser.hpp"

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

		vector<XMLNamespace> SaxHandler::namespaceVector;

    XMLNamespaceVector& SaxHandler::getXMLNamespacePtrVector()
			{
				return namespaceVector;
			}


    const string &SaxHandler :: getType () const
		{
			static string type = "SAX";

			return type;
		}

    SaxHandler::SaxHandler() {
        init();
    }

    SaxHandler::SaxHandler(const SaxHandler& s) :
        documentPtr(s.documentPtr),
        nspacePtr(s.nspacePtr),
        ownsNspacePtr (false),
        elementNameStack(s.elementNameStack),
        currentElementName(s.currentElementName),
        parent(s.parent),
        pcdata(s.pcdata),
        currentAtts(s.currentAtts),
        debug(s.debug)
        {
    }

    void SaxHandler::init() {
        debug = false;
        documentPtr = 0;

        nspacePtr = new XMLNamespace;
        ownsNspacePtr = true;

        currentElementName = CH_EMPTY;
        parent = CH_EMPTY;
        pcdata = CH_EMPTY;
        currentAtts.clear();
        elementNameStack.clear();
    }

    SaxHandler::~SaxHandler() {
			if (ownsNspacePtr)
				{
					delete nspacePtr;
				}
    }

    void SaxHandler::debugOutput(const string &s) const
    {
        if (debug)
        	{
						cerr << s;
					}
    }

    void SaxHandler::debugOutput(const char * const text_p) const
    {
        if (debug)
        	{
						cerr << text_p;
					}
    }




// ------------------------ SAX events -------------------
    // SAX-like call back
    bool SaxHandler::startDocument() {
    // clear all internals
        currentElementName = CH_EMPTY;

        if (!nspacePtr)
        	{
						nspacePtr = new XMLNamespace;
						ownsNspacePtr = true;
					}
        debugOutput("<!--startDocument-->");
			return true;
    }

    // SAX-like call back
    bool SaxHandler::endDocument()
    	{
        debugOutput("<!--endDocument-->");
    		return true;
    	}


    void SaxHandler::startElement(const string &name, AttributeVector &atts) {

        processAttributes(atts);
        startElement(nspacePtr->getURL(), name, nspacePtr->getPrefix(), atts);
    }


// main start routine
    void SaxHandler::startElement(const string &namespaceURI, const string &localName, const string &prefix, AttributeVector &atts) {
        if (getDebug()) {
            cerr << "<";
            if (nspacePtr->getPrefix() != CH_EMPTY) {
                cerr << nspacePtr->getPrefix() << ":";
            }
            cerr << localName << flush;
            for (unsigned int i = 0; i < atts.size(); i++) {
                cerr << " " << atts[i].first << "=\"" << atts[i].second << "\"";
            }
            cerr << ">" << flush;
        }
        if (currentElementName != CH_EMPTY) {
            elementNameStack.push_back(currentElementName);
        }
        parent = currentElementName;
				currentElementName = localName;
				XMLUtils :: trim (currentElementName);

        currentAtts = atts;
        pcdata = CH_EMPTY;
        if (getDebug()) {
            cerr << "<!-- SAX end start --> " << endl;
        }
    }

    void SaxHandler::processAttributes(AttributeVector &atts) {
        for (AttributeVector::size_type i = 0; i < atts.size(); ++i) {
            string name = atts[i].first;
            if (!Parser::isXMLName(name)) {
                XMLUtils::cmlError("invalid XML name: " + name);
            } else if (name.substr(0,5) == X_XMLNS) {
                processXMLNamespace(name.substr(5), atts[i].second);
            }
        }
    }

    void SaxHandler::processXMLNamespace(const string &name, const string &value) {
#ifndef INCHI_LIB
        //cerr << "defined new namespace: " << name.substr(1) << " = " << value << endl;
        // replaced with the following 11-17-2005 DT
        cerr << "defined new namespace: " << ((name.substr(0,1)==CH_EMPTY)? CH_EMPTY : name.substr(1)) << " = " << value << endl;
        cerr << "not yet implemented " << endl;
#endif
    }

    void SaxHandler::endElement(const string &name) {
        if (debug) {
            cerr << "sax end element: " << name << endl;
        }
        endElement(nspacePtr->getURL(), name, nspacePtr->getPrefix());
    }

    void SaxHandler::endElement(const string &namespaceURI, const string &localName, const string &prefix) {
        if (getDebug()) {
            ostream& os = cerr;
            cerr << "</";
            if (nspacePtr->getPrefix() != CH_EMPTY) {
                os << ":" << nspacePtr->getPrefix();
            }
            os << localName << ">" << endl;
        }
        StringVector strings;

				string name = localName;

        XMLUtils :: trim (name);
        if (name != currentElementName) {
            XMLUtils::cmlError("SH unbalanced tags at: " + name);
        }
        if (elementNameStack.size() > 0) {
            currentElementName = elementNameStack.back();
            elementNameStack.pop_back();
            parent = (elementNameStack.size() > 0) ? elementNameStack.back() : CH_EMPTY;
        } else {
//            cerr << "ran off element stack" << endl;
        }

        pcdata = CH_EMPTY;
    }

    void SaxHandler::characters(string & s) {
			pcdata =XMLUtils::processXMLEntities(s);
       XMLUtils::trim(pcdata);
        if (pcdata != CH_EMPTY) {
            debugOutput("PCDATA: " + pcdata);
        }
    }

    void SaxHandler::processingInstruction(const string &target, const string &data) {
      //	cerr << "PI: " << target << CH_SPACE << data << endl;
    }


	 XMLNamespace* SaxHandler::getXMLNamespacePtr() const {return nspacePtr;}
	 void SaxHandler::setXMLNamespacePtr(XMLNamespace* s) {nspacePtr = s;}

	 const AttributeVector& SaxHandler::getCurrentAtts() const {return currentAtts;}
	 void SaxHandler::setCurrentAtts(const AttributeVector& av) {currentAtts = av;}

	 const string &SaxHandler::getPCDATA() const {return pcdata;}
	 void SaxHandler::setPCDATA(const string& s) {pcdata = s;}

	 Document* SaxHandler::getDocumentPtr() const {return documentPtr;}
	 void SaxHandler::setDocumentPtr(Document* d) {documentPtr = d;}

	 string SaxHandler::getCurrentElementName() const {return currentElementName;}
	void SaxHandler::setCurrentElementName(const string& en) {currentElementName = en;}

	 const string &SaxHandler::getParent() const {return parent;}
	void SaxHandler::setParent(const string& en) {parent = en;}

	const StringVector& SaxHandler::getElementNameStack() const {return elementNameStack;}


}


