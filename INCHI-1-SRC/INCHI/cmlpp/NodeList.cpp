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
#include "NodeList.hpp"

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


namespace CML {

NodeList :: NodeList (Node *nodes_p, const unsigned int num_nodes)
	: nl_length (num_nodes)
{
	Node **node_pp = nl_nodes_pp = new Node * [nl_length];

	for (unsigned int i = nl_length; i > 0; -- i, ++ node_pp, ++ nodes_p)
		{
			*node_pp = nodes_p;
		}
}


NodeList :: NodeList (vector <Node *> &nodes)
{
	nl_length = nodes.size ();
	Node **node_pp = 	nl_nodes_pp = new Node * [nl_length];
	Node **other_node_pp = & (nodes [0]);

	for (unsigned int i = nl_length; i > 0; -- i, ++ node_pp, ++ other_node_pp)
		{
			(*node_pp) = (*other_node_pp);
		}
}


NodeList :: ~NodeList ()
{
	delete [] nl_nodes_pp;
}


unsigned int NodeList :: getLength () const
{
	return nl_length;
}


Node *NodeList :: item (unsigned int index) const
{
	if (index < nl_length)
		{
			return (* (nl_nodes_pp + index));
		}
	else
		{
			return 0;
		}
}



/********
NodeList :: NodeList ()
{}


NodeList :: NodeList (vector <Node *> &nodes)
{
	Node **node_pp = & (nodes [0]);

	for (unsigned int i = nodes.size (); i > 0; -- i, ++ node_pp)
		{
			cerr << "pushing " << *node_pp << " " << typeid (**node_pp).name () << endl;
			nl_nodes.push_front (*node_pp);

		}
}


NodeList :: ~NodeList ()
{}


unsigned int NodeList :: getLength () const
{
	return (nl_nodes.size ());
}


Node *NodeList :: item (unsigned int index)
{
	if (index <= nl_nodes.size ())
		{
			list <Node *> :: iterator ite;

			ite = nl_nodes.begin ();

			for ( ; index > 0; -- index)
				{
					++ ite;
				}

			return *ite;
		}
	else
		{
			return 0;
		}
}
*******/

} // namespace CML
