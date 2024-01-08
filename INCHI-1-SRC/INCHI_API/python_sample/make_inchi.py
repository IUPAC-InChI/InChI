#!/usr/bin/python
#
# International Chemical Identifier (InChI)
# Version 1
# Software version 1.02
# November 30, 2008
# Developed at NIST
# 
# The InChI library and programs are free software developed under the
# auspices of the International Union of Pure and Applied Chemistry (IUPAC);
# you can redistribute this software and/or modify it under the terms of 
# the GNU Lesser General Public License as published by the Free Software 
# Foundation:
# http://www.opensource.org/licenses/lgpl-license.php
# 
 

"""
	Test the INCHI API

"""

import os
import sys
import string

from ctypes import *
from optparse import OptionParser

from lightsdf import *
from PyINCHI import *


def make_pystring(p):
	s = ""	
	try:
		for c in p:
			if c=='\0':
				break
			s = s + c
	except:
		pass
	return s



def process_sdf_record(insdf,irec,r,fw,fl,iopts,baux,getinchi,freeinchi,getikey,ikey,ixtra,inchilogmess, nerr):


	# Skip if no structure field present

	if isstruct(r)==0:
		return

	guessname=''


	# Parse SD/MOL fields

	info = convert_MDL_structure_as_text_to_preobject(r['Structure_as_text'],guessname)
	name = info['Name']
	if (name==''):	
		j = irec + 1
		name = insdf.fname+'#'+`j`


	
	nat = info['Natoms']
	nbonds = info['Nbonds']
#	fw.write('***************************************************************************\n')
#	fw.write('RECORD %-d Name = %-s\n' %  (irec+1,name) )
#	if (fl!=sys.stdout):
#		fl.write('RECORD %-d Name = %-s\n' %  (i+1,name) )



	# Make array of inchi_Atom (storage space) anf resp. pointers

	iatoms = (inchi_Atom * nat) ()
	piatoms = (POINTER(inchi_Atom) * nat) ()


	# Fill inchi_Atom's by SDF record atom block content

	atoms = info['Atoms']
	ia = -1
	valences = []
	for a in atoms:

		ia = ia + 1
		atname, charge, (x,y,z) , stereo_pairity, hydrogen_count, stereo_care, valence, h0_designator, reac_component_type, reac_component_num, atom_mapping_num, inv_ret_flag, exact_change_flag, dd = a
		x,y,z  = (x,y,z) 
		valences.append(valence)

		# Make inchi_Atom

		elname = (c_byte * PYINCHI_ATOM_EL_LEN) (PYINCHI_ATOM_EL_LEN*0)
		num_iso_H = (c_byte * (PYINCHI_NUM_H_ISOTOPES+1)) ((PYINCHI_NUM_H_ISOTOPES+1)*0)
		neighbor = (c_short * PYINCHI_MAXVAL) (PYINCHI_MAXVAL*0)
		bond_type = (c_byte * PYINCHI_MAXVAL) (PYINCHI_MAXVAL*0)
		bond_stereo = (c_byte * PYINCHI_MAXVAL) (PYINCHI_MAXVAL*0)

		i = 0
		for c in atname:
			elname[i] = ord(c)
			i = i + 1
		isotopic_mass = 0
            	if dd != 0:
                	isotopic_mass = dd + PYINCHI_ISOTOPIC_SHIFT_FLAG
		radical = 0
		if charge == 4:
                	radical = 2 #doublet
		else:
			if charge != 0:
				charge = 4 - charge
            
		iatom = inchi_Atom(
				x, y, z, 
			    	neighbor, # to be filled later
				bond_type, # to be filled later
				bond_stereo, # to be filled later
				elname, 
			    	0, # num_bonds, to be filled later
			    	num_iso_H, # to be filled later
			    	isotopic_mass, 
			    	radical, 
			    	charge
			    	)


		iatoms[ia] = iatom
		pia = pointer(iatoms[ia])
		piatoms[ia] = pia



	# Get bond block
	# push all bonding info to inchi_Atom structures

	bonds = info['Bonds']
	for b in bonds:
		atom1, atom2, type, stereo, topology,reac_center = b
		# fw.write('(%-d,%-d)\ttype=%-d\tster=%-d\ttopo=%-d\treac=%-d\n' %  b )
		j = atom1-1
		k = atom2-1
		# both atnums must be valid
		if ( (j>=0) and (j<nat) and (k>=0) and (k<nat) ):
			nbj = iatoms[j].num_bonds
			iatoms[j].neighbor[nbj] = k
				# getinchi does not like duplicated bonds
			iatoms[j].bond_type[nbj] = type
			iatoms[j].bond_stereo[nbj] = stereo
			iatoms[j].num_bonds = nbj + 1		

                	if (stereo == 1) or (stereo == 4) or (stereo ==  6):
				stereo = -stereo

			nbk = iatoms[k].num_bonds
			iatoms[k].neighbor[nbk] = j
			iatoms[k].bond_type[nbk] = type
			iatoms[k].bond_stereo[nbk] = stereo
			iatoms[k].num_bonds = nbk + 1
			


	# Process property lines

	lines = info['Prop']
	FirstChargeOrRadical = 1
	for line in lines:
		try:
			code = 	line[3:6]
			vals = string.split(line[7:],' ')		
			nf = len(vals)
			for k in range(nf-1,-1,-1):
				if vals[k] =='': del vals[k]
			nv = int(vals[0])
			if nv<1:
				continue
			if code=="ISO":
				#for j in range(nat):
					#iatoms[j].isotopic_mass = 0
				for j in range(1,2*nv+1,2):
					ia = int(vals[j]) - 1
					imass = int(vals[j+1])
					if (ia<0) or (ia>nat):
						continue
					iatoms[ia].isotopic_mass = imass

			elif code=="CHG":
				if (FirstChargeOrRadical > 0):
					for j in range(nat):
						iatoms[j].charge = 0
						iatoms[j].radical = 0
                                	FirstChargeOrRadical = 0
				for j in range(1,2*nv+1,2):
					ia = int(vals[j]) - 1
					ichg = int(vals[j+1])	
					if (ia<0) or (ia>nat):
						continue
					iatoms[ia].charge = ichg

			elif code=="RAD":
				if (FirstChargeOrRadical > 0):
					for j in range(nat):
						iatoms[j].charge = 0
						iatoms[j].radical = 0
                                	FirstChargeOrRadical = 0
				for j in range(1,2*nv+1,2):
					ia = int(vals[j]) - 1
					irad = int(vals[j+1])	#1=singlet, 2=doublet, 3=triplet
				        if irad==1:
						#InChI should treat singlet (additional lone pair) as a triplet which reduces numH by 2
						irad = 3 
					if (ia<0) or (ia>nat):
						continue
					iatoms[ia].radical = irad

		except:
			pass

	for ia in range(0,nat):

		v = valences[ia]

		if v>0:
	        	# need to use valences if any to find out number of H

	
			nBondsValence = 0
		        nNumAltBonds = 0
            		for a1 in range (0, iatoms[ia].num_bonds):
                		if (iatoms[ia].bond_type[a1] < 4):
                    			nBondsValence = nBondsValence + iatoms[ia].bond_type[a1]
                		else:
                    			nNumAltBonds = nNumAltBonds + 1

            		if (nNumAltBonds == 2):
				nBondsValence = nBondsValence + 3
            		elif (nNumAltBonds == 3):
                		nBondsValence = nBondsValence + 4
            		elif (nNumAltBonds > 0):
                		#Error: wrong number of aromatic bonds
				#In this simplified code we do not provide any diagnostics
        		        nBondsValence = nBondsValence + nNumAltBonds

			#number of H
			if (nBondsValence == 0) and (valences[ia] == 15):
                		numH = 0
            		else:
                		numH = valences[ia] - nBondsValence
                		if (numH < 0):
					numH = 0

            		iatoms[ia].num_iso_H[0] = numH

		else:
			# let inchi hydrogenize automatically
			iatoms[ia].num_iso_H[0] = -1

	# inchi dislike bond repetitions, clean the adjacncy lists...
	for ia in range(0,nat):
		nbr = iatoms[ia].neighbor[:]
		bty = iatoms[ia].bond_type[:]
		bst = iatoms[ia].bond_stereo[:]
		nbs = iatoms[ia].num_bonds
		iatoms[ia].num_bonds = 0
		for nb in range(0,nbs):
			if (nbr[nb]) > ia:
				iatoms[ia].neighbor[iatoms[ia].num_bonds] = nbr[nb]
				iatoms[ia].bond_type[iatoms[ia].num_bonds] = bty[nb]
				iatoms[ia].bond_stereo[iatoms[ia].num_bonds] = bst[nb]
				iatoms[ia].num_bonds = iatoms[ia].num_bonds + 1				


	# Compose inchi_Input

	istereo0D = (inchi_Stereo0D * 1) ()
	pistereo0D = (POINTER(inchi_Stereo0D) * 1) ()

	iinput = inchi_Input(
			iatoms,		# inchi-style atoms
		    	istereo0D,      # stereo0D (empty)
		    	iopts, 		# command line switches
		    	nat, 		# natoms
		    	0		# num_stereo0D
			) 	


	# Prepare inchi_Output

	szInChI  = create_string_buffer(1) # NB: will be reallocd by inchi dll
	szIKey = create_string_buffer(256) # 
	szXtra1 = create_string_buffer(256) # 
	szXtra2 = create_string_buffer(256) # 
	szAuxInfo  = create_string_buffer(1) 
	szMessage = create_string_buffer(1) 
	szLog = create_string_buffer(1) 
	ioutput = inchi_Output( cast(pointer(szInChI), POINTER(c_char)),
			cast(szAuxInfo, POINTER(c_char)),
			cast(szMessage, POINTER(c_char)),
			cast(szLog, POINTER(c_char)) )



	# Call DLL function(s)

	result = getinchi(byref(iinput), byref(ioutput))  # 0 = okay, 
							  # 1 => warning, 
							  # 2=>error, 
							  # 3=>fatal


	# Process results

	si = make_pystring(ioutput.szInChI)
	fw.write('%-s\n' % si)

	if (result!=0): 
		# problem!
		if (fl!=sys.stdout):
			fl.write('RECORD %-d Name = %-s\n\t' %  (irec+1,name) )
		if (result==1):
			fl.write('Warning : ')
		else:
			fl.write('InChI creation error (%-d) : ' % result)
			nerr = nerr + 1
		fl.write('%-s\n' % make_pystring(ioutput.szMessage))

	if (result<=1):
		# no error or just warnings
		if (ikey==1):
			result1 = getikey(ioutput.szInChI, ixtra, ixtra, szIKey, szXtra1, szXtra2)
			if (result1!=0):
				if (fl!=sys.stdout):
					fl.write('RECORD %-d Name = %-s\t' %  (irec+1,name) )
				# special case: reversal of InChI failed
				# InChIKey is computed, but the warning issued
				if (result1==11):
					fl.write('Warning: InChI reversal gets mismatching InChI string\n')
					fw.write('InChIKey=%-s\n' % make_pystring(szIKey))
				else:
					fl.write('Key calculation error (%-d)\n' % result1)


			else:
				fw.write('InChIKey=%-s\n' % make_pystring(szIKey))
				if (ixtra==1):
					fw.write('XHash1=%-s\n' % make_pystring(szXtra1))
					fw.write('XHash2=%-s\n' % make_pystring(szXtra2))


	if baux:
		fw.write('%-s\n' % make_pystring(ioutput.szAuxInfo))
	if inchilogmess=="":
		inchilogmess = make_pystring(ioutput.szLog)


	# Deallocate memory reallocated by inchi dll 

	freeinchi(byref(ioutput))


	return (inchilogmess, nerr)







if __name__ == "__main__":



	print "This Python demo program reads SD file and calls InChI library"
	print "(libinchi.dll/libinchi.so.1) functions to generate "
	print "InChI strings and InChIKey codes."
	print "Note: the example is provided for illustrative purposes only."
	print

	parser = OptionParser(usage="""\
Usage: %prog [options]
""")
	parser.add_option('-i', '--input',
                      	type='string', action='store',
                      	help='name of input SD file (required)')
	parser.add_option('-o', '--output',
                      	type='string', action='store',
                      	help='name of output file (default=inchi_out.txt)')
	parser.add_option('-l', '--log',
                      	type='string', action='store',
                      	help='name of log file (errors/warnings; default=stdout)')
	parser.add_option('-s', '--start_record',
                      	type='int', action='store',
                      	help='starting number of record to be converted')
	parser.add_option('-e', '--end_record',
                      	type='int', action='store',
                      	help='number of the last record to be converted')
	parser.add_option('-x', '--aux', dest='aux', 
			help='print auxilary info', default=False, action='store_true')
	parser.add_option('-k', '--key', dest='key', 
			help='calculate InChIKey', default=False, action='store_true')
	parser.add_option('-z', '--xtra', dest='xtra', 
			help='calculate extra hash complementing InChIKey', default=False, action='store_true')
	parser.add_option('-p', '--inchi_options', 
                      	type='string', action='store',
			help='string with InChI options')

	opts, args = parser.parse_args()

	if not opts.input:
		parser.print_help()
		sys.exit(1)
	fname = opts.input

	if opts.output:		
		fwrite = opts.output
	else:			
		fwrite = "inchi_out.txt"
	try:
		fw = open(fwrite,'wt')
	except:
		print "COUD NOT OPEN OUTPUT FILE"
		exit(2)

	if opts.inchi_options:
		iopts = opts.inchi_options
	else:
		iopts = ""

	if opts.log:
		try:
			fl = open(opts.log,'wt')
		except:
			print "COUD NOT OPEN LOG FILE"
			exit(3)
	else:
		fl = sys.stdout
	if not opts.aux:
		baux = 0
	else:
		baux = 1

	if not opts.key:
		ikey = 0
	else:
		ikey = 1

	if not opts.xtra:
		ixtra = 0
	else:
		ixtra = 1

	startr = 0
	if opts.start_record:
		startr = opts.start_record
	else:
		startr = -1
	if opts.end_record:
		endr = opts.end_record
	else:
		endr = -1


        insdf = SDF_file()
        if not insdf.open(fname):
                print '\n* IO fatal error *'
                sys.exit(4)


	libname = ''
	opsys = sys.platform
	if (opsys[:3]=='win'):
		libname = 'libinchi.dll'
	else:
		if (opsys[:5]=='linux'):
			libname = '/usr/lib/libinchi.so.1'
	try:
		libinchi = cdll.LoadLibrary(libname)
		getinchi = libinchi.GetINCHI
		freeinchi = libinchi.FreeINCHI
		getikey = libinchi.GetINCHIKeyFromINCHI
	except:
		print '** ERROR ** Could not access InChI library', libname
		sys.exit()
	inchilogmess = ""
	timing = time.clock()
	i = -1
	nerr = 0
        while 1: 
		try:
			rec = insdf.readnext()
                	if rec == None:
               			break
			else:
       		        	i = i + 1
				if (i+1<startr):
					continue
				if (endr>0):
					if (i+1>endr):
						break
				print 'RECORD #',i+1
				inchilogmess, nerr = process_sdf_record(insdf,i,rec,
						   fw,fl,
						   iopts,baux,
						   getinchi,freeinchi,getikey, 
						   ikey, ixtra,
						   inchilogmess, nerr)
		except:
			print "***ERROR IN RECORD",i+1,", SKIPPED"
			fw.write('***ERROR IN RECORD %-ld", SKIPPED\n'% i)

	 
	fl.write('\n[Used InChI options:\n%-s]\n' %  inchilogmess )
	

	fl.write( '\nProcessed records: %-ld \nTotal errors: %-ld' % (i+1,nerr) )	
	
	timing = time.clock() - timing
	fl.write( '\nELAPSED TIME: %-.2f SEC' % timing )
