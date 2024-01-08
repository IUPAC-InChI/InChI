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


#ifndef CMLBASE_HPP
#define CMLBASE_HPP

#include <iostream>
#include <string>
#include <vector>

/*
#include "XMLUtils.hpp"
#include "chars.hpp"
#include "xmlchars.hpp"
#include "cmlnames.hpp"
*/

#include "Element.hpp"
#include "symbols.hpp"
#include "Parser.hpp"
#include "CMLSaxHandler.hpp"
#include "CMLDocument.hpp"

using namespace std;
namespace CML {

//class Parser;
//class CMLSaxHandler;

//class CMLDocument;

/* base class for all elements */
class CMLPP_API CMLBase : public Element {

public:

	const static string EMPTY;
	const static int UNSET_INT;
	const static double UNSET_DOUBLE;

	CMLBase();
	CMLBase(CMLDocument* documentPtr, string tagName);
	CMLBase(const CMLBase& base);
	virtual ~CMLBase();

	virtual void endSAX(SaxHandler* saxPtr);

	virtual bool setUntypedAttribute(const string& attName, const string& attVal);
	void setUntypedAttributes();

	// used by a number of subclasses (torsion, etc.)
	static void getAtomRefs(StringVector::size_type size, StringVector &v, const string &atomRefString);

	void /*CMLBase::*/ outputCML1Builtin(ostream& os, const string& dataType, const string& attName, const Serializer* const serializerPtr) const;
	void /*CMLBase::*/ outputCML1Builtin(ostream& os, const string& dataType, const string& attName, const string& attValue, const Serializer* const serializerPtr) const;

	void outputCML1Array(ostream& os, const string arrayType, const string attName, const Serializer* const serializerPtr, const vector <CMLBase*> &basePtrVector) const;

	virtual void outputCML2Attribute(ostream& os, const string &attName, const vector <CMLBase*> &basePtrVector) const;

	virtual string getDictRef() const;
	virtual void setDictRef(const string& d);
	virtual bool dictRefIsSet() const;

	virtual string getId() const;
	virtual void setId(const string& i);
	virtual bool idIsSet() const;

	virtual string getTitle() const;
	virtual void setTitle(const string& t);
	virtual bool titleIsSet() const;

protected:

	string dictRef;
	bool dictRefSet;

	string idd;
	bool idSet;

	string title;
	bool titleSet;

	void convertBuiltinToAttributes(CMLBase* basePtr);
	void convertBuiltinArrayToVector(CMLBase* basePtr, string& name, string& array);
};

}

#endif //CMLBASE_HPP

