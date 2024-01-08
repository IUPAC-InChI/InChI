'''
Yet another Python (3) to InChI Software Library interface
'''
import os
import sys
import string
from ctypes import *
#
#
#
#
##############################################################################
#
#
#
##############################################################################
def what_platform():
    ''' Returns 'Windows' or 'Linux' '''
    opsys = sys.platform
    if (opsys[:3]=='win'):
        return "Windows"
    elif (opsys[:5]=='linux'):
        return "Linux"
    else:
        return "Unsupported"
#
#
#
def load_inchi_library(path):
    #
    platform = what_platform()
    print ("Platform =",platform)
    libname = ""
    if (platform=="Windows"):
        libname = path + "libinchi.dll"
    elif (platform=="Linux"):
        libname = path + "libinchi.so.1" #"/usr/lib/libinchi.so.1"
    else:
        return None
    #
    print (libname)
    try:
        print ("Loading InChI Software library", libname)
        libinchi = cdll.LoadLibrary(libname)
        print ("..loaded.")
        makeinchi = libinchi.MakeINCHIFromMolfileText
        freeinchi = libinchi.FreeINCHI
        getikey = libinchi.GetINCHIKeyFromINCHI
    except:
        return None
    #
    if not makeinchi:
        return None
    if not freeinchi:
        return None
    if not getikey:
        return None
    #
    return (makeinchi, freeinchi, getikey)
#
#
#
##############################################################################
#
#
#
class inchi_Output(Structure):
    # zero-terminated C-strings allocated by GetINCHI()
    # to deallocate all of them call FreeINCHI() (see below)
    _fields_ = [("szInChI", POINTER(c_char) ), 
                ("szAuxInfo", POINTER(c_char) ), 
                ("szMessage", POINTER(c_char) ), 
                ("szLog", POINTER(c_char) ) ]  
#
#
#
##############################################################################
#
#
#
def calc_inchikey( sinchi, calc_xkey, inchi_api ):
    #
    sikey = sxtra1 = sxtra2 = ""
    makeinchi, freeinchi, getikey = inchi_api
    #
    c_sinchi = c_char_p(sinchi.encode('utf-8'))
    #
    # Fixed length buffers
    #
    szIKey  = create_string_buffer(256) 
    szXtra1 = create_string_buffer(256) 
    szXtra2 = create_string_buffer(256) 
    #
    ikey_result = getikey(c_sinchi, calc_xkey, calc_xkey, szIKey, szXtra1, szXtra2)
    if ikey_result==0 or ikey_result==11:   
        # 11 is a special case: 
        # reversal of InChI failed, InChIKey is computed with warning issued
        sikey = cast( szIKey, c_char_p).value.decode("utf-8")
    if (calc_xkey==1):
        sxtra1 = cast( szXtra1, c_char_p).value.decode("utf-8")
        sxtra2 = cast( szXtra2, c_char_p).value.decode("utf-8")
    #
    return ( ikey_result, sikey, sxtra1, sxtra2 )
##############################################################################
#
#
#
def inchify_molfile( molfile, inchi_options, show_auxinfo, calc_key, calc_xkey, inchi_api ):
    #
    makeinchi, freeinchi, getikey = inchi_api
    sinchi = slog = smessage = saux = sikey = sxtra1 = sxtra2 = ""
    inchi_result = ikey_result = ierr = 0
    #
    # Prepare inchi_Output
    #
    #
    # Fixed length buffers
    #
    szIKey  = create_string_buffer(256) 
    szXtra1 = create_string_buffer(256) 
    szXtra2 = create_string_buffer(256) 
    #
    # Variable lenght buffers (will be reallocated by libinchi)
    #
    szInChI     = create_string_buffer(1) 
    szAuxInfo   = create_string_buffer(1) 
    szMessage   = create_string_buffer(1) 
    szLog       = create_string_buffer(1) 
    ioutput = inchi_Output( szInChI, szAuxInfo, szMessage, szLog )
    #
    #Make InChI
    #
    c_molfile = c_char_p(molfile.encode('utf-8'))
    c_inchi_options = c_char_p(inchi_options.encode('utf-8'))
    #
    inchi_result = makeinchi( c_molfile, c_inchi_options, byref(ioutput) )
    #
    sinchi = cast(ioutput.szInChI, c_char_p).value.decode("utf-8")
    slog = cast(ioutput.szLog, c_char_p).value.decode("utf-8").strip()
    smessage = cast(ioutput.szMessage, c_char_p).value.decode("utf-8").strip()
    saux = cast(ioutput.szAuxInfo, c_char_p).value.decode("utf-8")
    #
    #
    if inchi_result > 1:
        ierr = 1
    else:
        # no error or just warning ==> may compute InChIKey if requested
        if calc_key==1:
            ikey_result = getikey(ioutput.szInChI, calc_xkey, calc_xkey, szIKey, szXtra1, szXtra2)
            if ikey_result==0 or ikey_result==11:   
                # 11 is a special case: 
                # reversal of InChI failed, InChIKey is computed with warning issued
                sikey = cast( szIKey, c_char_p).value.decode("utf-8")
                if (calc_xkey==1):
                    sxtra1 = cast( szXtra1, c_char_p).value.decode("utf-8")
                    sxtra2 = cast( szXtra2, c_char_p).value.decode("utf-8")
    #
    #
    #
    # Deallocate memory reallocated by inchi dll 
    #
    # Note the trick with None (Python's NULL) assignnment to pointers
    # which have not been actually allocated (related strings are zero-length) 
    # within C library libinchi. 
    # We passed Python buffer_strings casted to char *, not real pointers
    # so they could not be inited with NULL (as they would be in C)... 
    # So if say szLog or szMessage were actually not used in libinchi.dll 
    # and were not allocated, they should be massaged to become NULL at the end, 
    # in order to avoid unnecessary/illegal freeing upon call of freeinchi().
    #
    # -- all above is valid on no error.
    #
    #print (ioutput.szInChI, ioutput.szAuxInfo, ioutput.szLog, ioutput.szMessage)
    #print ( inchi_result, "{" + sinchi +"}, {" + slog +"}, {" + smessage +"}, {" + saux + "}" )
    #
    #
    #
    if len(sinchi)==0:
        ioutput.szInChI = None # Python's NULL
    if len(slog)==0:
        ioutput.szLog = None
    if len(smessage)==0:
       ioutput.szMessage = None
    freeinchi(byref(ioutput))
    return ( inchi_result, ikey_result, sinchi, slog, smessage, saux, sikey, sxtra1, sxtra2, ierr)
