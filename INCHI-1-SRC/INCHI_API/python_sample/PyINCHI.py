#!/usr/bin/python
#
# International Chemical Identifier (InChI)
# Version 1
# Software version 1.04
# September 9, 2011
# 
# The InChI library and programs are free software developed under the
# auspices of the International Union of Pure and Applied Chemistry (IUPAC).
# Originally developed at NIST. Modifications and additions by IUPAC 
# and the InChI Trust.
# 
# IUPAC/InChI-Trust Licence No.1.0 for the 
# International Chemical Identifier (InChI) Software version 1.04
# Copyright (C) IUPAC and InChI Trust Limited
# 
# This library is free software; you can redistribute it and/or modify it 
# under the terms of the IUPAC/InChI Trust InChI Licence No.1.0, 
# or any later version.
# 
# Please note that this library is distributed WITHOUT ANY WARRANTIES 
# whatsoever, whether expressed or implied.  See the IUPAC/InChI Trust 
# Licence for the International Chemical Identifier (InChI) Software 
# version 1.04, October 2011 ("IUPAC/InChI-Trust InChI Licence No.1.0") 
# for more details.
# 
# You should have received a copy of the IUPAC/InChI Trust InChI 
# Licence No. 1.0 with this library; if not, please write to:
# 
# The InChI Trust
# c/o FIZ CHEMIE Berlin
# 
# Franklinstrasse 11
# 10587 Berlin
# GERMANY
# 
#  or email to: ulrich@inchi-trust.org.
#  





"""
        Interface to INCHI library  (used by InChI generation example)
	
	The implementation is very 'light' and is provided for
	illustrative purposes only.

"""


import os
import sys
import string

from ctypes import *


PYINCHI_MAXVAL = 20
PYINCHI_ATOM_EL_LEN = 6
PYINCHI_NUM_H_ISOTOPES = 3
#this flag means isotopic shift relative to avg. atw, not abs. isotopic mass
PYINCHI_ISOTOPIC_SHIFT_FLAG  = 10000


##########################################################
# 0D - S T E R E O  (if no coordinates given)
##########################################################

class inchi_Stereo0D(Structure):
	_fields_ = [("neighbor", c_short * 4), 	# 4 atoms always
		    ("central_atom", c_short), 	# central tetrahedral atom or a central */
                       				# atom of allene; otherwise NO_ATOM */
		    ("type", c_byte), 		# inchi_StereoType0D
		    ("parity", c_byte)] 	# inchi_StereoParity0D: may be a combination of two parities: */
						# ParityOfConnected | (ParityOfDisconnected << 3), see Note above */
	def dump(self):
       		print "\tDump of inchi_Stereo0D structure"
		print '\t\t neighbor: ',
		for nbr in self.neighbor:
			print nbr,
		print
		print '\t\t central_atom: ', self.central_atom
		print '\t\t type: ', self.type
		print '\t\t parity: ', self.parity






##########################################
# inchi_Atom
##########################################

class inchi_Atom(Structure):
	_fields_ = [("x", c_double),	# atom coordinates 
		    ("y", c_double),
		    ("z", c_double),
		    # connectivity 
		    ("neighbor", c_short * PYINCHI_MAXVAL), 	# adjacency list: ordering numbers of the adjacent atoms, >= 0
		    ("bond_type", c_byte * PYINCHI_MAXVAL), 	# inchi_BondType 
		    # 2D stereo
		    ("bond_stereo", c_byte * PYINCHI_MAXVAL), 	# inchi_BondStereo2D; negative if the sharp end points to another atom 
		    # other atom properties
		    ("elname", c_byte * PYINCHI_ATOM_EL_LEN), 	# zero-terminated chemical element name: "H", "Si", etc.
		    ("num_bonds", c_short ), # number of neighbors, bond types and bond stereo in the adjacency list
		    ("num_iso_H", c_byte * (PYINCHI_NUM_H_ISOTOPES+1)),	# implicit hydrogen atoms
							     	# [0]: number of implicit non-isotopic H
							     	# (exception: num_iso_H[0]=-1 means INCHI adds implicit H automatically),
							     	# [1]: number of implicit isotopic 1H (protium),
							     	# [2]: number of implicit 2H (deuterium),
							     	# [3]: number of implicit 3H (tritium) 
		    ("isotopic_mass", c_short ), 		# 0 => non-isotopic; isotopic mass or 10000 + mass - (average atomic mass)
		    ("radical", c_byte  ), 			# inchi_Radical
		    ("charge", c_byte  )] 			# positive or negative; 0 => no charge

	def fdump(self, fw):
		fw.write('\t{\t --- Dump of inchi_Atom structure ---\n')
		s = ""
		for sy in self.elname:
			s = s + chr(sy)
		fw.write('\t\t element: %-s \n' % s )
		fw.write('\t\t charge: %-d radical: %-d isotopic_mass: %-d\n' % 	
			(self.charge, self.radical, self.isotopic_mass) )
		fw.write('\t\t num_bonds: %-d\n' %  self.num_bonds)
		fw.write('\t\t neighbor: ')
		for nbr in self.neighbor:
			fw.write(' %-d' % nbr)
		fw.write('\n')
		fw.write('\t\t bond_types: ')
		for bt in self.bond_type:
			fw.write(' %-d ' % bt)
		fw.write('\n')
		fw.write('\t\t bond_stereos: ')
		for bs in self.bond_stereo:
			fw.write(' %-d' % bs)
		fw.write('\n')
		fw.write('\t\t num_iso_H: ')
		for ni in self.num_iso_H:
			fw.write(' %-d' % ni)
		fw.write('\n\t} \n')

	def dump(self):
		self.fdump(sys.stdout)


##########################################
# Structure -> InChI, GetINCHI() 
##########################################

class inchi_Input(Structure):
	# the caller is responsible for the data allocation and deallocation
	_fields_ = [("atom", POINTER(inchi_Atom)), 	  # actually, pointer to array of inchi_Atom pointers
		    ("stereo0D", POINTER(inchi_Stereo0D)), # actually, pointer to array of inchi_Stereo0D
		    ("szOptions", c_char_p), # InChI options: space-delimited; each is preceded by '/' or '-' 
		    ("num_atoms", c_int), #c_short), # number of atoms in the compound < 1024 
		    ("num_stereo0D", c_short)]  # number of 0D stereo elements 





# /* InChI -> Structure, GetStructFromINCHI() */
# typedef struct tagINCHI_InputINCHI {
#     /* the caller is responsible for the data allocation and deallocation */
#     char *szInChI;     /* InChI ASCIIZ string to be converted to a strucure */
#     char *szOptions;   /* InChI options: space-delimited; each is preceded by */
#                        /* '/' or '-' depending on OS and compiler */
# } inchi_InputINCHI;




##########################################################
# InChI -> Structure, GetStructFromINCHI() 
##########################################################

class inchi_InputINCHI(Structure):
	_fields_ = [("szInChI", c_char_p), 	# InChI ASCIIZ string to be converted to a strucure 
		    ("szOptions", c_char_p)]	# InChI options: space-delimited; each is preceded by
                       				# '/' or '-' depending on OS and compiler */

#  the caller is responsible for the data allocation and deallocation



##########################################################################
#	inchi_Output
##########################################################################

class inchi_Output(Structure):
	# zero-terminated C-strings allocated by GetINCHI()
	# to deallocate all of them call FreeINCHI() (see below)

	_fields_ = [("szInChI", POINTER(c_char) ), # c_char_p
		    ("szAuxInfo", POINTER(c_char) ), 
		    ("szMessage", POINTER(c_char) ), 
			("szLog", POINTER(c_char) ) ]  # c_char_p)]

	def dump(self):
       		print "\tDump of inchi_Output structure"
		print '\t\t',self.szInChI
		print '\t\t',self.szAuxInfo
		print '\t\t',self.szMessage
		print '\t\t',self.szLog



##########################################################################
#	InChI -> Structure
##########################################################################

#class inchi_OutputStruct(Structure):
#	# 4 pointers are allocated by GetStructFromINCHI() 
#	# to deallocate all of them call FreeStructFromINCHI()
#	_fields_ = [("atom", c_long), 	  # actually, pointer to array of inchi_Atom
#		    ("stereo0D", c_long), # actually, pointer to array of inchi_Stereo0D
#
#	_fields_ = [("atom", POINTER(inchi_Atom)), 	  # actually, pointer to array of inchi_Atom
#		    ("stereo0D", POINTER(inchi_Stereo0D)), # actually, pointer to array of inchi_Stereo0D
#
#
#		    ("szMessage", c_char_p), # Error/warning ASCIIZ message
#		    ("szLog", c_char_p), # log-file ASCIIZ string, contains a human-readable list
#					 # of recognized options and possibly an Error/warning message
#
#
#		    ("num_stereo0D", c_short)]  # number of 0D stereo elements 
#
# 
# 
# typedef struct tagINCHI_OutputStruct {
#     inchi_Atom     *atom;         /* array of num_atoms elements */
#   inchi_Stereo0D *stereo0D;     /* array of num_stereo0D 0D stereo elements or NULL */
#     AT_NUM          num_atoms;    /* number of atoms in the structure < 1024 */
#     AT_NUM          num_stereo0D; /* number of 0D stereo elements */
#     char           *szMessage;    /* Error/warning ASCIIZ message */
#     char           *szLog;        /* log-file ASCIIZ string, contains a human-readable list */
#                                   /* of recognized options and possibly an Error/warning message */
#     unsigned long  WarningFlags[2][2]; /* warnings, see INCHIDIFF in inchicmp.h */
#                                        /* [x][y]: x=0 => Reconnected if present in InChI otherwise Disconnected/Normal
#                                                   x=1 => Disconnected layer if Reconnected layer is present
#                                                   y=1 => Main layer or Mobile-H
#                                                   y=0 => Fixed-H layer
#                                         */
# }inchi_OutputStruct;
