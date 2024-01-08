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
#include "Parser.hpp"

using namespace std;
namespace CML {

    Parser::Parser() {
        init();
    }

    Parser::~Parser() {
    }

    void Parser::init() {
        debug = false;
        saxHandlerPtr = 0;
        inComment = false;
    }

    void Parser::setSaxHandler(SaxHandler* shPtr) {
        saxHandlerPtr = shPtr;
        setDebug(shPtr->getDebug());
    }

    bool Parser::parse(InputWrapper &input) {
        if (debug) {
            cerr << "parser start " << endl;
        }
        if (saxHandlerPtr == 0) {
            XMLUtils::cmlError("No sax handler in parser - bug ");
            return false;
        }
        string token = CH_EMPTY;
        size_t lt = 0;
        size_t rt = 0;
        int lineCount = 0;

        bool lookForOpenTag = true;

        if (saxHandlerPtr->startDocument())
					{
						string buff = "";
						while (input.getLine(&buff)) {
								lineCount++;
								if (debug) {
										cerr << "parser: " << lineCount << ": " << buff << endl;
								}
				// omit whitespace lines
								string temp (buff);
								XMLUtils::trim (temp);
								if (temp == CH_EMPTY) {
										if (debug) {
												cerr << "skipped empty... " << endl;
										}
										continue;
								}
				// normalize Newlines to SPACE
								if (token != CH_EMPTY) token += CH_SPACE;
								for (;;) {
										if (inComment) {
												lt = buff.find(E_COMMENT);
												if (lt > buff.size()) {
														token += buff;
														buff = CH_EMPTY;
														break;
												} else {
														inComment = false;
														buff = buff.substr(lt+1);
														continue;
												}
										}
										if (lookForOpenTag) {
												lt = buff.find(CH_LANGLE);
				// not found, more input...
												if (lt > buff.size()) {
														token += buff;
														buff = CH_EMPTY;
														break;
												} else {
				// found start of Tag
														token += buff.substr(0,lt);
				// process inter-Tag characters
														saxHandlerPtr->characters(token);
														buff = buff.substr(lt);
														token = CH_EMPTY;
														lookForOpenTag = false;
												}
										} else {
												rt = buff.find(CH_RANGLE);
				// not found, more input...
												if (rt > buff.size()) {
														token += buff;
														buff = CH_EMPTY;
														break;
												} else {
				// found end
														string ss = buff.substr(0,rt+1);
														token += ss;
														tag(token);
														buff = buff.substr(rt+1);
														token = CH_EMPTY;
														lookForOpenTag = true;
												}
										}
										if (buff == CH_EMPTY) break;
								}
						}
						saxHandlerPtr->endDocument();

						return true;
				}
			else
				{
#ifndef INCHI_LIB
					cerr << "Couldn't parse document" << endl;
#endif
					return false;
				}
    }

    // process anything in balanced <...>
    void Parser::tag(string &s) {
        // attributes
        AttributeVector atts;

        string name;
        string::size_type l = s.length();
        string sl (s);
				XMLUtils ::  toLowerCase (sl);
    // XML declaration
        if (sl.substr(0, 5) == X_XMLDECL) {
            if (debug) {
                cerr << "XML Declaration " << s << endl;
            }
            if (s.substr(l-2, 2) == E_PI) {
                string ss = s.substr(5, l-7);
                Parser::splitAttributes(ss, atts);
                string standalone = XMLUtils::getAttributeValue(atts, X_STANDALONE);
                if (standalone == X_NO) {
                  XMLUtils::cmlError("cannot process standalone='no' yet");
                }
                string version = XMLUtils::getAttributeValue(atts, X_VERSION);
                if (version != X_VERSION_NUMBER) {
                  string msg = "XML version must be: ";
                  string msg1 = X_VERSION_NUMBER;
                  XMLUtils::cmlError(msg + msg1);
                }
                string encoding = XMLUtils::getAttributeValue(atts, X_ENCODING);

								XMLUtils :: toLowerCase (encoding);

                if (encoding != "utf-8" && encoding != CH_EMPTY) {
                  XMLUtils::cmlError("Cannot support encoding: " + encoding);
                }
                if (debug) {
                    cerr << "end XML Declaration " << endl;
                }
            } else {
              XMLUtils::cmlError("Bad XML declaration: " + s);
            }
    // DOCTYPE is not processed
        } else if (s.substr(0,9) == X_DOCTYPE) {
            if (debug) {
                cerr << "XML Doctype " << s << endl;
            }
            if (s.find("[") <= s.size()) {
                        XMLUtils::cmlError("cannot process internal subset of DOCTYPE " + s);
            } else {
              //			cerr << "DOCTYPE info ignored" << endl;
            }
    // comments are ignored
        } else if (s.substr(0,4) == S_COMMENT) {
            if (debug) {
                cerr << "comment " << s << endl;
            }
            if (s.substr(l-3, 3) == E_COMMENT) {
                inComment = false;
    //			cerr << "Comment ignored: " << s << endl;
            } else {
                inComment = true;
                XMLUtils::cmlError("Bad comment: " + s);
            }
    // Processing instructions
        } else if (s.substr(0,2) == S_PI) {
            if (debug) {
                cerr << "PI " << s << endl;
            }
            if (s.substr(l-2, 2) == E_PI) {
                s = s.substr(2, l-4);
                string::size_type idx = s.find(CH_SPACE);
                string target = (idx < s.size()) ? s.substr(0, idx) : s;

                string data;
								if (idx < s.size ())
									{
										data = s.substr (idx);
										XMLUtils::trim (data);
									}
								else
									{
										data = CH_EMPTY;
									}


						    saxHandlerPtr->processingInstruction(target, data);
            } else {
              XMLUtils::cmlError("Bad PI: " + s);
            }
    // CDATA sections - not processed
        } else if (s.substr(0,9) == S_CDATA) {
            if (debug) {
                cerr << "CDATA " << s << endl;
            }
            if (s.substr(l-3, 3) == E_CDATA) {
//                saxPtr->getPCDATA() += s.substr(9, l-12);
            } else {
              XMLUtils::cmlError("Bad CDATA: " + s);
            }
    // end Tag
        } else if (s.substr(1,1) == CH_SLASH) {
            if (debug) {
                cerr << "endTag " << s << endl;
            }
            saxHandlerPtr->endElement(s.substr(2, l-3));
    // empty Tag
        } else if (s.substr(l-2, 1) == CH_SLASH) {
            if (debug) {
                cerr << "emptyTag " << s << endl;
            }
            name = startTag(s.substr(1,l-3));
            saxHandlerPtr->endElement(name);
    // start Tag
        } else {
            if (debug) {
                cerr << "startTag " << s << endl;
            }
            startTag(s.substr(1, l-2));
        }
    }

    string Parser::startTag(const string &str) {
        if (debug) {
            cerr << "startTag (" << str << ")" << endl;
        }
        AttributeVector atts;
        atts.clear();

				string s (str);
				XMLUtils::trim (s);

        if (s.find(CH_AMP) <= s.size()) {
          XMLUtils::cmlError("CML reader cannot process entity references (sorry)..." + s);
        }
        string ss = s;
        string name = CH_EMPTY;
        string::size_type idx = s.find(CH_SPACE);
        if (idx > s.size()) {
            name = s;
            s = CH_EMPTY;
        } else {
            name = s.substr(0, idx);
						s = s.substr (idx + 1);
            XMLUtils :: trim (s);
        }
        Parser::splitAttributes(s, atts);
        if (!Parser::isXMLName(name)) {
          XMLUtils::cmlError("invalid XML name: " + name);
        }
        if (debug) {
            cerr << "sax startElement(" << name << ")" << " type" << saxHandlerPtr->getType() << endl;
        }
        saxHandlerPtr->startElement(name, atts);
        if (debug) {
            cerr << "exit startTag (" << s << ")" << endl;
        }
        return name;
    }


    // splits into name and value
    void Parser::splitAttributes(string &s, AttributeVector &atts) {
        Attribute att;

        while (true) {
            string::size_type idx = s.find(CH_EQUALS);
            if (idx > s.size()) {

                if (XMLUtils::trim(s) != CH_EMPTY) {
                  XMLUtils::cmlError("Bad attribute at " + s);
                }
                break;
            }
            att.first = s.substr(0, idx);
            XMLUtils::trim(att.first);
            s = s.substr(idx+1);
            XMLUtils::trim (s);
            if (s.length() < 2) {
              XMLUtils::cmlError("Bad attribute value: " + s);
                break;
            }
    // quote or X_APOS
            string quoter = s.substr(0, 1);
            if (quoter != CH_QUOTE && quoter != CH_APOS) {
              XMLUtils::cmlError("Unquoted attribute value: " + s);
                break;
            }
            s = s.substr(1);
            idx = s.find(quoter);
            if (idx > s.size()) {
              XMLUtils::cmlError("Unbalanced quotes in attribute value: " + s);
                break;
            }
            string temp = s.substr(0, idx);
            att.second = XMLUtils::processXMLEntities(temp);
            atts.push_back(att);
            s = s.substr(idx+1);
            XMLUtils::trim(s);
            if (XMLUtils::trim(s) == CH_EMPTY) break;
        }
    }

    // normalizes string
    string Parser::getNormalizedString(const string &s) {
        return getNormalizedString (s.c_str());
    }

    // normalizes string
    string Parser::getNormalizedString(const char* ch) {
        bool inWhite = true;
        bool start = true;
        string s = CH_EMPTY;
        for (unsigned int i = 0;; i++) {
            char c = ch[i];
            if (c == 0) break;
            if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
                inWhite = true;
            } else {
                if (start) {
                    start = false;
                } else {
                    if (inWhite) s += " ";
                }
                inWhite = false;
                s += c;
            }
        }
        return s;
    }

   void Parser::writePCDATA(ostream&os, const string &value) {
        os << XMLUtils::escapeXMLEntities(value);
    }

    bool Parser::isXMLName(const string &n) {
        bool ok = true;;

        char *str = const_cast <char *> (n.c_str ());
        char c = *str++;
    // first character must be a-zA-Z_
        if (!( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' )) {
            ok = false;
        }
        while ((c = *str++) != 0) {
            if (c >= '0' && c <= '9') {
            } else if (c >= 'a' && c <= 'z') {
            } else if (c >= 'A' && c <= 'Z') {
            } else if (c == '_' || c == ':' || c == '-' || c == '.') {
            } else {
                ok = false;
            }
        }
        if (!ok) {
          XMLUtils::cmlError("invalid XML name: " + n);
        }
        return ok;
    }

 }

