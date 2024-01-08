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
#include "Serializer.hpp"

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



string Serializer::getType() const
{
	return "BASE";
}



Serializer::Serializer()
	:	doctype (CH_EMPTY),
		useDeclaration (false),
		pretty (true),
		nspacePtr (0),
		debug (false)
{
	nspacePtr = new XMLNamespace;
}

Serializer::Serializer (const string &doctype, bool useDeclaration, bool pretty, XMLNamespace* nspacePtr, bool debug)
	:	doctype(doctype),
		useDeclaration(useDeclaration),
		pretty(pretty),
		nspacePtr(nspacePtr),
		debug(debug)
{}

Serializer::Serializer(const Serializer& s)
	:	doctype(s.doctype),
		useDeclaration(s.useDeclaration),
		pretty(s.pretty),
		nspacePtr(0),
		debug(s.debug)
{
	if (s.nspacePtr)
		{
			nspacePtr = new XMLNamespace (*s.nspacePtr);
		}
}

Serializer &Serializer :: operator = (Serializer &other)
{
	if (this != &other)
		{
			doctype = other.doctype;
			useDeclaration = other.useDeclaration;
			pretty = other.pretty;
			debug = other.debug;

			if (nspacePtr)
				{
					if (other.nspacePtr)
						{
							*nspacePtr = * (other.nspacePtr);
						}
					else
						{
							delete nspacePtr;
							nspacePtr = 0;
						}
				}
			else
				{
					if (other.nspacePtr)
						{
							nspacePtr = new XMLNamespace (* (other.nspacePtr));
						}
				}
		}

	return *this;
}


Serializer::~Serializer()
{
	delete nspacePtr;
}


ostream &operator << (ostream &stm, const Serializer &ser_r)
{
	stm << "{Serializer: ";
	stm << "doctype = " << ser_r.doctype << "/";
	stm << "useDeclaration = ";
	stm << ((ser_r.useDeclaration) ? "true" : "false");
	stm << "/";
	stm << "pretty = ";
	stm << ((ser_r.pretty) ? "true" : "false");
	stm << "/";
	stm << "nspacePtr = " << * (ser_r.nspacePtr) << "/";
	stm << "debug = ";
	stm << ((ser_r.debug) ? "true" : "false");
	stm << "}";

	return stm;
}



/** outputs XML header
*
* outputs declaration, doctype if requested
*/
void Serializer::outputHeader(ostream &os) const
{
	if (useDeclaration)
		{
			os << CH_LANGLE << CH_QUERY << X_XML << CH_SPACE << X_VERSION << CH_EQUALS <<  CH_QUOTE << X_VERSION_NUMBER << CH_QUOTE << CH_QUERY << CH_RANGLE << endl;
		}

	// not right as we have to get the root element
	if (doctype != CH_EMPTY)
		{
			os << X_DOCTYPE << CH_SPACE << C_MOLECULE << CH_SPACE << X_SYSTEM << CH_QUOTE << doctype << CH_QUOTE << CH_RANGLE << endl;
		}
}

void Serializer::writeAttribute(ostream &os, const string& name, const int value) const
{
	os << CH_SPACE << name << CH_EQUALS << CH_QUOTE << value << CH_QUOTE;
}

void Serializer::writeAttribute(ostream &os, const string& name, const double value) const
{
	os << CH_SPACE << name << CH_EQUALS << CH_QUOTE << value << CH_QUOTE;
}

void Serializer::writeAttribute(ostream &os, const string& name, const string& value) const
{
	string value1 (value);
	XMLUtils::trim(value1);

	if (value1 != CH_EMPTY)
		{
			value1 = XMLUtils::escapeXMLEntities(value1);
			os << CH_SPACE << name << CH_EQUALS << CH_QUOTE << value1 << CH_QUOTE;
		}
}

void Serializer::writeStartTagStart(ostream& os, const string& name) const
{

	os << CH_LANGLE << nspacePtr->getPrefixString() << name;
}

void Serializer::writeStartTagEnd(ostream& os) const
{
	os << CH_RANGLE;
}

void Serializer::writeEndTag(ostream& os, const string& name) const
{
	os << CH_LANGLE << CH_SLASH << nspacePtr->getPrefixString() << name << CH_RANGLE << endl;
}

void Serializer::writeCombinedTagEnd(ostream& os) const
{
	os << CH_SLASH << CH_RANGLE << endl;
}


void Serializer::setDoctype(const string& d)
{

	doctype = d;
}

const string &Serializer::getDoctype() const
{

	return doctype;
}

void Serializer::setDeclaration(bool b)
{

	useDeclaration = b;
}

bool Serializer::getDeclaration() const
{

	return useDeclaration;
}

void Serializer::setPretty(bool b)
{

	pretty = b;
}

bool Serializer::getPretty() const
{

	return pretty;
}

void Serializer::setXMLNamespacePtr(XMLNamespace* const n)
{

	nspacePtr = n;
}

const XMLNamespace* Serializer::getXMLNamespacePtr() const
{

	return nspacePtr;
}

void Serializer::setDebug(bool b)
{

	debug = b;
}

bool Serializer::getDebug() const
{

	return debug;
}

}	// namespace CML

