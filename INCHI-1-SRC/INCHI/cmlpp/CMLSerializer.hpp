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


#ifndef CMLSERIALIZER_HPP
#define CMLSERIALIZER_HPP

#include <iostream>
#include <string>
#include <vector>
#include "symbols.hpp"

#include "Serializer.hpp"
#include "XMLNamespace.hpp"

using namespace std;
namespace CML {

class CMLPP_API CMLSerializer : public Serializer {

	friend CMLPP_API ostream &operator << (ostream &stm, const CMLSerializer &ser_r);


public:

	static const CMLSerializer * const getDefaultSerializerPtr();
	static void freeDefaultSerializerPtr ();

	static const XMLNamespace * const getCML1DtdPtr ();
	static const XMLNamespace * const getCML2CorePtr ();
	static void  freeCML1DtdPtr ();
	static void  freeCML2CorePtr ();


	//! Constructor
	CMLSerializer();
	CMLSerializer (const string& doctype, const bool& useDeclaration, const bool& pretty, XMLNamespace* nspacePtr, const bool& debug, const string &cmlType, const bool& array);

	CMLSerializer(const CMLSerializer& s);

	//! Destructor
	virtual ~CMLSerializer();

	virtual string getType() const;

	virtual void setBaseSerializer(Serializer* serializerPtr);

	/** outputs XML header
	*
	* outputs declaration, doctype if requested
	*/

	void writeBuiltin(ostream& os, const string& name, int value);
	void writeBuiltin(ostream& os, const string& name, double value);
	void writeBuiltin(ostream& os, const string& name, const string& value);

	virtual void setCmlType(const string& c);
	const string &getCmlType() const;

	//    void setCmlTypeInt(int c) {cmlTypeInt = c;}
	//    int getCmlTypeInt() {return cmlTypeInt;}

	void setArray(const bool& b);
	bool getArray() const;


	XMLNamespace* getXMLNamespace(string& type);


protected:
	static CMLSerializer* defaultCMLSerializerPtr;
	static XMLNamespace *cml1DtdPtr;
	static XMLNamespace *cml2CorePtr;

	string cmlType;
	bool array;

private:
	void init();


};

} //end namespace

#endif // CMLSERIALIZER_HPP
