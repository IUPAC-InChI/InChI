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


#ifndef NODE_LIST_HPP
#define NODE_LIST_HPP

#include <list>
#include <vector>

#include "symbols.hpp"

#include "Node.hpp"

using namespace std;

namespace CML {

class CMLPP_API NodeList {

public:
	NodeList (Node *nodes_p, unsigned int num_nodes);
	NodeList (vector <Node *> &nodes);
	~NodeList ();

	unsigned int getLength () const;
	Node *item (unsigned int index) const;

private:
	//list <Node *> nl_nodes;
	Node **nl_nodes_pp;
	unsigned int nl_length;
};

}

#endif // NODE_LIST_HPP

