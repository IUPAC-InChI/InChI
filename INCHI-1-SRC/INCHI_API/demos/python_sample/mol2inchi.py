import os
import sys
import string
import gzip
from optparse import OptionParser
from yapyinchi import *
#
#
#
##############################################################################
def show_record_results(rst, fout, flog):
    #
    inchi_result, ikey_result, sinchi, slog, smessage, saux, sikey, sxtra1, sxtra2, ierr = rst
    #
    if inchi_result != 0:
        if (flog!=sys.stdout):
            flog.write('\n  RECORD %-d\t' %  (num) )
        if (inchi_result==1):
            flog.write('Warning : ')
        else:
            flog.write('InChI creation error (%-d) : ' % inchi_result)
        flog.write('%-s' % smessage )
    if inchi_result <=1 :
        # no error or just warning
        #print('%-s' % sinchi)
        fout.write('%-s\n' % sinchi)
        # ... may compute InChIKey
        if calc_key==1:
            if ikey_result!=0:
                if (fl!=sys.stdout):
                    flog.write('RECORD %-d\t' %  (num) )
                # special case: reversal of InChI failed
                # InChIKey is computed, but the warning issued
                if (ikey_result==11):
                    flog.write('Warning: InChI reversal gets mismatching InChI string\n')
                    fout.write('InChIKey=%-s\n' % sikey)
                else:
                    flog.write('Key calculation error (%-d)\n' % ikey_result)
            else:
                #print('InChIKey=%-s' % sikey )
                fout.write('InChIKey=%-s\n' % sikey )
                if (calc_xkey==1):
                    print('XHash1=%-s' % sxtra1 )
                    print('XHash1=%-s' % sxtra2 )
                    fout.write('XHash1=%-s\n' % sxtra1 )
                    fout.write('XHash2=%-s\n' % sxtra2 )
    if show_auxinfo:
        fout.write('%-s\n' %  saux )
    return


##############################################################################
def retire(code):
    print ("Something went wrong...")
    
    if ( code==1 ):
        print ("Invalid command line")
    elif ( code==2 ):
        pass
    elif( code==3 ):
        print ("Unsupported platform")
    elif( code==4 ):
        print ("Could not access InChI library")
    elif( code==5 ):
        print ("Could not open input file")
    elif( code==6 ):
        print ("Could not open output file")
    elif( code==7 ):
        print ("Could not open log file")
    else:
        pass
    
    sys.exit(code)



##############################################################################
def cl_parser():
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
        retire(1)
    else:
        inname = opts.input
    if opts.output:
        outname = opts.output
    else:
        outname = "inchi_out.txt"    
    if opts.log:
        logname = opts.log
    else:
        logname = "" 

    inchi_options = ""
    platform = what_platform()
    if platform=='Windows':
        inchi_options = "/W60"
    else:
        if platform=='Linux':
            inchi_options = "-W60"
 
    if opts.inchi_options:
        inchi_options = inchi_options + opts.inchi_options
    if not opts.aux:
        show_auxinfo = 0
    else:
        show_auxinfo = 1
    if not opts.key:
        calc_key = 0
    else:
        calc_key = 1
    if not opts.xtra:
        calc_xkey = 0
    else:
        calc_xkey = 1
    if opts.start_record:
        startr = opts.start_record
    else:
        startr = -1
    if opts.end_record:
        endr = opts.end_record
    else:
        endr = -1
    return [inname, outname, logname, inchi_options, show_auxinfo, calc_key, calc_xkey, startr, endr]
##############################################################################
def open_files( inname, outname, logname ):
    try:
        if  os.path.splitext(inname)[-1]==".gz":
            f = gzip.open(inname)
        else:
            f = open(inname, "rb")
    except:
        return ( None, 5 )            
    #
    try:
        fout = open(outname, 'w')
    except:
        return ( None, 6 )                        
    if not fout:
        return ( None, 6 )                        
    #
    try:
        if ( logname==""):
            flog = sys.stderr
        else:
            flog = open(logname, 'w')
    except:
        return ( None, 7 )                                    
    if not flog:
        return ( None, 7 )                        

    return ( (f, fout, flog), 0)



##############################################################################
def banner():
    print ("This Python demo program reads SD file and calls InChI library")
    print ("(libinchi.dll/libinchi.so.1) function MakeINCHIFromMolfileText()")
    print ("to generate InChI string (also generates InChIKey).")
    print ("Note: the example is provided for illustrative purposes only.\n")


##############################################################################
def clean_and_make_list_of_records(text):
    try:
        text = text.replace("\r\n", "\n")
        # Split string by '$$$$'
        content = text.split("$$$$")
        # Remove surplus empty record appearing after the last $4  (if any)
        if ( len(content[-1])<2 ):    # \n
            content = content[:-1]
    except: 
        retire(97)    
    # Clean up segments: remove surplus head linefeed in records 1-n start
    # ( it appears as we split at '$$$$' not '$$$$\n' )
    def del_head_lf(s):
        if s:
            if s[0]=="\n":
                s = s[1:]
        return s
    content = [content[0]] + [del_head_lf(item) for item in content[1:]]
    return content


##############################################################################
#
#   MAIN
#
##############################################################################


banner()

parameters = cl_parser()
if not parameters:
    retire(1)

inname, outname, logname, inchi_options, show_auxinfo, calc_key, calc_xkey, startr, endr = parameters    

inchi_api = load_inchi_library("")
if ( not inchi_api ):
    retire(4)  

print ("\nProcessing " + inname)

files, ierr = open_files( inname, outname, logname )
if not files:
    retire(ierr)

f, fout, flog = files


print ("\nReading, please wait..")
text = f.read()
print ("..done.")
text = text.decode('utf-8')

content = clean_and_make_list_of_records(text)

num = nerr = 0
for record in content:
    num = num + 1
            
    if (num<startr):
        continue
    if (endr>0):
        if (num>endr):
            break
            
    flog.write ( "\nRecord # %-ld (%-ld chars)" % (num, len(record)) )
    #print ('{' + record + '}')
            
    results = inchify_molfile( record, inchi_options, show_auxinfo, calc_key, calc_xkey, inchi_api )
            
    show_record_results( results, fout, flog )
    
    inchi_result, ikey_result, sinchi, slog, smessage, saux, sikey, sxtra1, sxtra2, ierr = results
    nerr = nerr + ierr

#print ( "\nProcessed %-ld records; had %-ld errors." % (num, nerr))
flog.write ( "\nCOMPLETED\nProcessed %-ld records with %-ld errors\n" % (num, nerr))
