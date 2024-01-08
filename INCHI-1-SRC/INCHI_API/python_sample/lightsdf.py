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


#===============================================================
#
# Light SDF reader object (used by InChI generation example)
#
#	Parsing MDL SD file records and collecting information
#	(atomic, bonding, etc.) for further use
#	The implementation is very 'light' and is provided for
#	illustrative purposes only.
#
#
#===============================================================


#---------------------------------------------------------------- 
def sint(s):
        """ smart convert to int """    
        ss = string.strip(s)
        if ss == '':
                return 0
        return int(ss)
#---------------------------------------------------------------- 
def sfloat(s):
        """ smart convert to float """  
        ss = string.strip(s)
        if ss == '':
                return 0.0
        return float(ss)
#---------------------------------------------------------------- 
def stripw(s):
        """ strip common-use whitespaces -
		leading, trailing, and within-string """  
        s1 = string.strip(s)
        s1 = string.replace(s1,'\n','')
        s1 = string.replace(s1,'\r','')
        s1 = string.replace(s1,'\t','')
        return s1
#---------------------------------------------------------------- 
def isstruct(record):
        """ check if structural data present in SDF record
                return 
                1 if structural data present or 
                0 - otherwise """

        if string.count(record['Structure_as_text'],'\n') < 5:
                return 0
        else:
                return 1
#---------------------------------------------------------------- 
def convert_MDL_structure_as_text_to_preobject(text,strname=''):
        """ convert structure in MDL's SDF record 
        	from textual representation to object """

        record = {}
	
        lines = string.split(text,'\n')

	record['Name'] = strname
	if (strname==''):
		name_line = stripw(lines[0])
		if (name_line!=''):
			record['Name'] = name_line

        counts_line = lines[3]

        try:
                natoms = sint(counts_line[:3])
                record['Natoms'] = natoms
        except:
                raise 'MDL_CT_ERROR_COUNTS',natoms
        try:
                nbonds = sint(counts_line[3:6])
                record['Nbonds'] = nbonds
        except:
                raise 'MDL_CT_ERROR_COUNTS',nbonds
        try:
                chiral_flag = sint(counts_line[12:15])
        except:
                raise 'MDL_CT_ERROR_COUNTS', chiral_flag
        try:
                nproplines = sint(counts_line[30:33])
		#print 'nproplines',nproplines
        except:
                raise 'MDL_CT_ERROR_COUNTS', nproplines


        # atom block

        atom_block = lines[4:4+natoms] 
        record['Atoms'] = []
        for line in atom_block:
                try:
                        x = sfloat(line[:10])
                        y = sfloat(line[10:20])
                        z = sfloat(line[20:30])
                        atname = string.strip(line[31:34])
                        dd = sint(line[34:36])
                        charge = sint(line[36:39])      
                        stereo_pairity = sint(line[39:42])
                        hydrogen_count = sint(line[42:45])
                        stereo_care = sint(line[45:48])
                        valence = sint(line[48:51])
                        h0_designator = sint(line[51:54])
                        reac_component_type = sint(line[54:57])
                        reac_component_num = sint(line[57:60])
                        atom_mapping_num = sint(line[60:63])
                        inv_ret_flag = sint(line[63:66])
                        exact_change_flag = sint(line[66:69])
			record['Atoms'].append( (atname, charge, (x,y,z) ,
						 stereo_pairity, 
						 hydrogen_count, 
						 stereo_care, 
						 valence, 
						 h0_designator, 
						 reac_component_type, 
						 reac_component_num, 
						 atom_mapping_num, 
						 inv_ret_flag, 
						 exact_change_flag,
						 dd ) )
                except:
                        raise 'MDL_CT_ERROR_ATOM'
                

        # bond block
        bond_block = lines[4+natoms:4+natoms+nbonds]
        record['Bonds'] = []
        for line in bond_block:
                try:
                        atom1 = sint(line[:3])
                        atom2 = sint(line[3:6])
                        type = sint(line[6:9])
                        stereo = sint(line[9:12])
                        topology = sint(line[15:18])
                        reac_center = sint(line[18:21])
                        record['Bonds'].append( (atom1, atom2, type, stereo, topology, reac_center) )
                except:
                        raise 'MDL_CT_ERROR_BOND'

        # prop block
        # NB: we do not refer to nproplines in counts line
	# instead, check possible property lines directly
        record['Prop'] = []
        for line in lines[4+natoms+nbonds:]:
                try:
			if line[:1]=="M":
				record['Prop'].append(line)
		except:
                        raise 'MDL_PROP_LINE_ERROR'

        return record





#===============================================================

"""
        Base class for light representation/processing of MDL SD file
	Light means no conversion to internal molobject representation,
	just parsing the text and collecting fields
"""

import sys, os, time, string


class SDF_file:
        """
        Base class for representation/processing of MDL's SD file

        SD file is represented as a collection (Python's list)
        of records with associated file name.

        Each record is a Python's dictionary, i.e. a series of
        pairs {key:value}.

        One key is pre-defined: 'Structure_as_text'
        Corresponding value, record['Structure_as_text'],
        contains structure representation as the text just as
        it stands in SDF file.

        All other keys (and values) are established as described
        in SDF file (key is that name which appeared in "> <key> ..."
        strings).       


        """
#---------------------------------------------------------------- 
        def __init__(self):
                """ SDF_file 
                        object constructor """
                
                self.records = []
                self.nrecords = 0
                self.fname = ''         
                self.fp = None
                self.keys = []


#---------------------------------------------------------------- 
        def open(self,fname):
                """ open SD file for reading """

                self.fname = fname

                try:
                        self.fp = open(fname,'rb')
                	self.nrecords = 0
	                self.records = []
                except:
                        return 0
                return 1


#---------------------------------------------------------------- 
        def read(self):
                """ read SDF file content """

                self.nrecords = 0
                self.records = []
                while 1:                
                        rec = self.readnext()
                        if rec == None:
                                break
                        #else:
                                self.nrecords = self.nrecords + 1
                                #print '[%-ld records]\r'%self.nrecords,
                                self.records.append(rec)
				#pass
                self.getkeys()
                #print


#---------------------------------------------------------------- 
        def readnext(self):
                """ read next record in SDF_file """

                rec = {}
                key = 'Structure_as_text'               
                rec[key] = ''

                # read line by line while no $$$$ sign occur

                while 1:

                        line = self.fp.readline()

                        if not line:
                                # EOF reached
                                # NB: last line in SDF should be empty
                                return None

                        if line[:4] == '$$$$' :
                                # have end-of-record line
                                rec[key] = string.rstrip(rec[key]) #remove trailing linefeed
                                break

                        if line[:1] == '>' :
                                # have field-name line
                                # remove trailing linefeed
                                rec[key] = string.rstrip(rec[key]) 
                                # extract field name
                                pos0 = string.find(line[1:],'<')
                                pos = string.find(line[pos0+1:],'>')
                                if pos < 1:
                                        key = 'empty'
                                else:
                                        key = line[pos0+2:pos0+pos+1]
                                rec[key] = ''

                        else:
                                # have field-content line
                                rec[key] = rec[key] + string.rstrip(line)+'\n'


#		if rec != None:
#			self.nrecords = self.nrecords + 1
#                       self.records.append(rec)

                return rec


#---------------------------------------------------------------- 
        def getkeys(self,sort=0):
                """ collect all the field names (keys) 
                        optionally, sort them lexicographically """

                self.keys = []
                for r in self.records:
                        rks = r.keys()
                        for rk in rks:
                                if rk not in self.keys:
                                        self.keys.append(rk)
                if sort>0:
                        self.keys.sort()
		return self.keys


#---------------------------------------------------------------- 
        def exportcsv(self,fwname, numbers=[], fields=[]):
                """ write content of specified fields (non-structural data) 
                        into comma-separated-values (.csv) format file """

                try:
                        fw = open(fwname,'wt')
                except:
                        return 0
                if  numbers == []:
                        numbers = range(self.nrecords)

                if fields == []:
                        fields = self.keys              


                # write field names in 1st line

                s = ''
                for k in fields:
                        
                        if k!='Structure_as_text':      # skip structure field
                                sk = k
                                if ' ' in sk:
                                        if '"' in sk:
                                                sk = string.replace(sk,'"','\'\'')
                                        s = s + '"' + sk + '",'
                                else:
                                        s = s + sk + ','
                fw.write('%-s\n'%s[:-1])

                # for each record write the fields

                for i in numbers:
                        r = self.records[i]
                        s = ''
                        for k in fields:
                                if k!='Structure_as_text':
                                        try:
                                                val = r[k]
                                                val = stripw(val)
                                        except:
                                                val = ''
                                        if ' ' in val or ',' in val:
                                                if '"' in val:
                                                        val = string.replace(val,'"','\'\'')
                                                s = s + '"' + val + '",'
                                        else:
                                                s = s + val + ','

                        fw.write('%-s\n'%s[:-1])

                return 1


#---------------------------------------------------------------- 
        def write(self,fwname,numbers=[]):
                """ write SDF file containing specified 
                        records  in specified order """

                try:
                        fw = open(fwname,'wt')
                except:
                        return 0
                if numbers == []:
                        numbers = range(self.nrecords)
                for i in numbers:
                        r = self.records[i]
                        #place structure at first
                        fw.write('%-s\n'%r['Structure_as_text'])
                        for k in r.keys():
                                if k!='Structure_as_text':
                                        fw.write('>  <%-s>\n'%k)
                                        fw.write('%-s\n\n'%r[k])
                        fw.write('$$$$\n')
                return 1


#---------------------------------------------------------------- 
        def getfieldduplicates(self,field):
                """ get duplicates by field idfield"""

                dup = {}
                ilist = range(self.nrecords) #all internal nos
                for i in ilist:
                        m = self.records[i]
                        try:
                                f = m[field]
                        except:
                                # no such field in this record
                                continue
                        try:
                                #if this id still here, add item
                                dup[f] = dup[f] + [i] 
                        except:
                                #if no this id presents, create entry
                                dup[f] = [i] 
                for dk in dup.keys():
                        if len(dup[dk])<2:
                                del dup[dk]
                return dup


