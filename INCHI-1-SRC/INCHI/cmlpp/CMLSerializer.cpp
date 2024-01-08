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
#include "CMLSerializer.hpp"

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

    XMLNamespace* CMLSerializer::cml1DtdPtr = 0;
    XMLNamespace* CMLSerializer::cml2CorePtr = 0;
    CMLSerializer* CMLSerializer::defaultCMLSerializerPtr = 0;


		const XMLNamespace* const CMLSerializer :: getCML1DtdPtr ()
			{
				if (!cml1DtdPtr)
					{
cout << "CMLSerializer :: getCML1DtdPtr ()";
						cml1DtdPtr = new XMLNamespace("cml", CML1_NAMESPACE);
					}

				return cml1DtdPtr;
			}

		const XMLNamespace* const CMLSerializer :: getCML2CorePtr ()
			{
cout << "CMLSerializer :: getCML2CorePtr ()";
				if (!cml2CorePtr)
					{
						cml2CorePtr = new XMLNamespace("cml", CML2_NAMESPACE);
					}

				return cml2CorePtr;
			}

    const CMLSerializer * const CMLSerializer::getDefaultSerializerPtr() {
cout << "CMLSerializer::getDefaultSerializerPtr()";
        if (defaultCMLSerializerPtr == 0) {
            defaultCMLSerializerPtr = new CMLSerializer;
            defaultCMLSerializerPtr->setCmlType(C_CML2);
            defaultCMLSerializerPtr->setArray(true);
        }
        return defaultCMLSerializerPtr;
    }

    void CMLSerializer :: freeDefaultSerializerPtr ()
    	{
				delete defaultCMLSerializerPtr;
			}

    void CMLSerializer :: freeCML1DtdPtr ()
    	{
				delete cml1DtdPtr;
			}

    void CMLSerializer :: freeCML2CorePtr ()
    	{
				delete cml2CorePtr;
			}


    string CMLSerializer::getType() const { return "CML"; }

    CMLSerializer::CMLSerializer() {
        init();
    }

    CMLSerializer::CMLSerializer(
        const string& doctype,
        const bool& useDeclaration,
        const bool& pretty,
        XMLNamespace* nspacePtr,
        const bool& debug,
        const string &cmlType,
        const bool& array)
        :
        Serializer(
        doctype,
        useDeclaration,
        pretty,
        nspacePtr,
        debug
        ),

        cmlType(cmlType),
        array(array)
        {
    }

    CMLSerializer::CMLSerializer(const CMLSerializer& s) :
        Serializer((Serializer) s),
        cmlType(s.cmlType),
        array(s.array)
        {
    }

    void CMLSerializer::init() {
        array = false;
        cmlType = C_CML2;
    }

    CMLSerializer::~CMLSerializer() {
    }

    void CMLSerializer::setBaseSerializer(Serializer* serializerPtr) {
        setDoctype(serializerPtr->getDoctype());
        setDeclaration(serializerPtr->getDeclaration());
        setXMLNamespacePtr(const_cast <XMLNamespace *> (serializerPtr->getXMLNamespacePtr()));
        setPretty(serializerPtr->getPretty());
        setDebug(serializerPtr->getDebug());
    }

    void CMLSerializer::setCmlType(const string& c) {
        cmlType = c;
    }

    void CMLSerializer::writeBuiltin(ostream& os, const string& name, int value) {
        os << "<" << getXMLNamespacePtr()->getPrefix() << "integer builtin=\"" <<  name << "\">" << value << E_TAGO <<
        getXMLNamespacePtr()->getPrefix() << "integer>" << endl;
    }

    void CMLSerializer::writeBuiltin(ostream& os, const string& name, double value) {
        os << "<" << getXMLNamespacePtr()->getPrefix() << "float builtin=\"" <<  name << "\">" << value << E_TAGO << getXMLNamespacePtr()->getPrefix() << "float>" << endl;
    }

    void CMLSerializer::writeBuiltin(ostream&os, const string& name, const string& value) {
        string value1 (value);

				XMLUtils :: trim (value1);

        if (value1 != CH_EMPTY) {
            value1 = XMLUtils::escapeXMLEntities(value1);
            os << "<" << getXMLNamespacePtr()->getPrefix() << "string builtin=\"" <<  name << "\">" << value1 << E_TAGO << getXMLNamespacePtr()->getPrefix() << "string>" << endl;
        }
    }


		ostream &operator << (ostream &stm, const CMLSerializer &ser_r)
			{
        stm << "{CMLSerializer: ";
        stm << "serializer = " << static_cast <const Serializer> (ser_r);
        stm << "/";
        stm << "cmlType = " + ser_r.cmlType + "/";
        stm << "array = ";
        stm << ((ser_r.array) ? "true" : "false");
        stm << "}";

				return stm;
			}



			const string &CMLSerializer::getCmlType() const {return cmlType;}

			//    void setCmlTypeInt(int c) {cmlTypeInt = c;}
			//    int getCmlTypeInt() {return cmlTypeInt;}

			void CMLSerializer::setArray(const bool& b) {array = b;}
			bool CMLSerializer::getArray() const {return array;}


}

