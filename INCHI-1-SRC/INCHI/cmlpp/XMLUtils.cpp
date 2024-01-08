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


#include <cstdlib> // Added 04-25-2010 (to work with gcc 4.3/4.4; thanks to Peter Linstrom)
#include <cstdio>  // Added 04-25-2010 (to work with gcc 4.3/4.4)

#include "pragma.hpp"
#include "XMLUtils.hpp"

using namespace std;
namespace CML {

//2010-02-07 const Attribute XMLUtils :: EMPTYATT("", "");

XMLUtils :: XMLUtils()
{}

XMLUtils :: ~XMLUtils()
{}

// resolves character entity references to the characters
string XMLUtils :: processXMLEntities(string &s)
{
	string s0 (s);
	string ss;

	for (;;)
		{
			string :: size_type idx = s.find(CH_AMP);
			if (idx >= s.length())
				{
					ss.append(s);
					break;
				}

			ss.append(s.substr(0, idx));
			s = s.substr(idx+1);
			idx = s.find(CH_COLON);

			if (idx >= s.length())
				{
					XMLUtils :: cmlError("entity without closing ; in :" + s0 + CH_COLON);
				}
			string e = s.substr(0, idx);

			if (e == X_QUOT)
				{
					ss.append(CH_QUOTE);
				}
			else if (e == X_APOS)
				{
					ss.append(CH_APOS);
				}
			else if (e == X_LT)
				{
					ss.append(CH_LANGLE);
				}
			else if (e == X_GT)
				{
					ss.append(CH_RANGLE);
				}
			else if (e == X_AMP)
				{
					ss.append(CH_AMP);
				}
			else if (e.substr(0, 1) == CH_HASH)
				{
					int i = atoi((char*)e.substr(1).c_str());

					if (i >= 32 && i < 256 || i == 9 || i==10 || i==13)
						{
							ss.append(1, (char)i);
						}
					else
						{
							XMLUtils :: cmlError("unsupported character: #" + i);
						}
				}
			else
				{
					XMLUtils :: skippedEntityMessage(e);
				}

			s = s.substr(idx+1);
		}

	return ss;
}

void XMLUtils :: skippedEntityMessage (const string &name)
{
//	cerr << "skipped entity: " << name << endl;
}

// replaces characters in string with their character entities
string XMLUtils :: escapeXMLEntities (const string &s)
{
	string ss;
	const unsigned char *c_ptr = reinterpret_cast <const unsigned char *> (s.c_str());
	const string :: size_type length = s.length ();

	for (string :: size_type i = length; i > 0;  -- i, ++ c_ptr)
		{
			switch (*c_ptr)
				{
					case '&':
					case '"':
					case '\'':
					case '<':
					case '>':
						ss.append("&");
						ss.append(reinterpret_cast <const char *> (c_ptr));
						ss.append(";");
					break;

					case ' ':
					case '\t':
					case '\n':
					case '\r':
						ss.append(1, *c_ptr);
					break;

					default:
						if (*c_ptr > ' ')
							{
								ss.append(1, *c_ptr);
							}
						else if (*c_ptr > 127)
							{
								ss.append(X_AMP_HASH);
								ss.append(reinterpret_cast <const char *> (c_ptr));
								ss.append(";");
							}
						else
							{
								XMLUtils :: cmlError("non-printing characters not suported: " + static_cast <int> (*c_ptr));
							}
					break;
				}
		}

	return ss;
}

// gets attribute of given name; CH_EMPTY if not found
int XMLUtils :: getAttributeIndex(const AttributeVector &atts, const string& attName)
{
	int i = atts.size() - 1;

	for ( ; i >= 0; -- i)
		{
			if (atts[i].first == attName)
				{
					return i;
				}
		}

	return -1;
}

// gets attribute of given name; CH_EMPTY if not found
string XMLUtils :: getAttributeValue(const AttributeVector &atts, const string& attName)
{
	string value = "";
	int idx = getAttributeIndex(atts, attName);
	if (idx >= 0)
		{
			value = atts[idx].second;
		}

	return value;
}

string XMLUtils :: createAttributeString(const string& attName, const string& attValue)
{
	string s = "";
	if (attValue != CH_EMPTY)
		{
			s = " " + attName + "=\"" + attValue + "\"";
		}
	return s;
}

void XMLUtils :: appendToArray(string &array, int value)
{
	if (array != CH_EMPTY)
		{
			array.append(CH_SPACE);
		}

	char ss[20];
	sprintf(ss, "%i", value);
	string s(ss);

	array.append(trim(s));
}

void XMLUtils :: appendToArray(string &array, double value)
{
	if (array != CH_EMPTY)
		{
			array.append(CH_SPACE);
		}

	char ss[20];
	sprintf(ss, "%f", value);
	string s(ss);
	array.append(trim(s));
}

void XMLUtils :: appendToArray(string &array, string &value)
{
	value = XMLUtils :: escapeXMLEntities(value);

	if (array != CH_EMPTY)
		{
			array.append(CH_SPACE);
		}

	array.append(trim(value));
}

void XMLUtils :: tokenizeStringToStringArray(StringVector &v, StringVector :: size_type n, const string &att)
{
	if (att == CH_EMPTY)
		{
			return;
		}

	StringVector sv;

	XMLUtils :: tokenize(sv, att, X_SPACE_NEWLINE);
	if (sv.size() != n)
		{
			XMLUtils :: cmlError("inconsistent stringarray attribute sizes: ");
#ifndef INCHI_LIB
			cerr << sv.size() << CH_SLASH << n << ":" << att << ":" << endl;
#endif
			return;
		}


	for (StringVector :: size_type i = 0; i < n; ++i)
		{
			//v[i] = sv[i];
			v.push_back(sv[i]);
		}
}

void XMLUtils :: tokenizeStringToIntArray(vector <int> &v, vector<int> :: size_type n, const string &att)
{
	if (att == CH_EMPTY)
		{
			return;
		}
	StringVector sv;

	XMLUtils :: tokenize(sv, att, X_SPACE_NEWLINE);
	if (sv.size() != n)
		{
			XMLUtils :: cmlError("inconsistent intarray attribute sizes: ");
#ifndef INCHI_LIB
			cerr << sv.size() << CH_SLASH << n << ":" << att << ":" << endl;
#endif
			return;
		}


	for (vector<int> :: size_type i = 0; i < n; ++i)
		{
			v.push_back(atoi((char*)sv[i].c_str()));
		}
}

void XMLUtils :: tokenizeStringToFloatArray(vector <double> &v, vector<double> :: size_type n, const string &att)
{
	if (att == CH_EMPTY)
		{
			return;
		}

	StringVector sv;

	XMLUtils :: tokenize(sv, att, X_SPACE_NEWLINE);
	if (sv.size() != n)
		{
			XMLUtils :: cmlError("inconsistent floatarray attribute sizes: ");
#ifndef INCHI_LIB
			cerr << sv.size() << CH_SLASH << n << ":" << att << ":" << endl;
#endif
			return;
		}


	for (vector<double> :: size_type i = 0; i < n; ++i)
		{
			v.push_back(atof(sv[i].c_str()));
		}
}

void XMLUtils :: printVector(StringVector& v, ostream& os)
{
	for (StringVector :: size_type i = 0; i < v.size(); ++i)
		{
			os << v[i] << CH_SPACE;
		}
}

void XMLUtils :: cmlError(const string msg)
{
#ifndef INCHI_LIB
	cerr << msg << endl;
#endif
}

void XMLUtils :: tokenize(StringVector &vcr, const string &s, const string& delimstr)
{
	tokenize(vcr, s, delimstr, -1);
}

void XMLUtils :: tokenize(StringVector &vcr, const string &str, const string& delimstr, const int limit)
{
	vcr.clear();
	size_t startpos=0,endpos=0;

	string s = str + CH_SPACE;

	int matched=0;
	unsigned int s_size = s.size();
	for (;;)
		{
			startpos = s.find_first_not_of(delimstr,startpos);
			endpos = s.find_first_of(delimstr,startpos);
			if (endpos <= s_size && startpos <= s_size)
				{
					string ss = s.substr(startpos,endpos-startpos);
					vcr.push_back(ss);

					matched++;
					if (limit > 0 && matched == limit)
						{
							startpos = endpos+1;
							vcr.push_back(s.substr(startpos,s_size));
							break;
						}
				}
			else
				{
					if (startpos < s_size)
						vcr.push_back(s.substr(startpos,s_size-startpos));
					break;
				}

			startpos = endpos+1;
		}
}

// trims whitespace from start and end.
string &XMLUtils :: trim (string &s)
{
	const char *c_p = s.c_str ();
	const char *ptr = c_p;

	int start = 0, length = s.length ();

	while (*ptr == ' ' || *ptr == '\n' || *ptr == '\t')
		{
			++ start;
			++ ptr;
		}

	ptr = c_p + length - 1;
	length -= start;

	while ((length > 0) && (*ptr == ' ' || *ptr == '\n' || *ptr == '\t'))
		{
			-- length;
			-- ptr;
		}

	if (length > 0)
		{
			s = s.substr (start, length);
		}
	else
		{
			s = s.substr (start);
		}

	return s;
}

void XMLUtils :: toLowerCase (string &s)
{
	for (int i = s.size () - 1; i >= 0; -- i)
		{
			s [i] = tolower (s [i]);
		}
}

void XMLUtils :: toUpperCase(string &s)
{
	for (int i = s.size () - 1; i >= 0; -- i)
		{
			s [i] = toupper (s [i]);
		}
}

bool XMLUtils :: isInStringVector(const StringVector &v, const string &s)
{
	int i = v.size() - 1;

	for ( ; i >= 0; -- i)
		{
			if (v[i] == s)
				{
					return true;
				}
		}

	return false;
}

}	// namespace CML

/* a simple function to perform a delayed load of cmlpp.dll */
/* for Window SEH to succeed it should not have any constructor affecting unwinding the stack  */
#if ( defined(WIN32) && defined(_MSC_VER) && _MSC_VER == 1200 )
extern "C" DLLDECLSPEC int __stdcall TestLoadDll( int n )
{
    return n;
}
#endif
