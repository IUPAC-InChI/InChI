# Comment out the next line to create so/dll only
CREATE_MAIN = 1

ifdef windir
# Under Win32/MinGW/gcc to add functions compliant to Pascal/stdcall calling
# conventions and provide all functionality present in the produced by the
# MS VC++ 6.0 libinchi.dll -- uncomment the next line
#   ADD_PASCAL_INCHI_CALLS = 1
else
# Uncomment the following line in case of Linux
# or define ISLINUX in command line: make ISLINUX=1
#   ISLINUX = 1
endif

# Linux fpic option: replace -fPIC with -fpic if the latter works
# Comment out "LINUX_Z_RELRO =" if -z relro is not supported
# These options are needed to avoid the following SELinux message:
# "Error: cannot restore segment prot after reloc: Permission denied"
# In addition, inchi.map restricts set of expoorted from .so
# functions to those which belong to InChI API
ifndef windir
LINUX_MAP = ,--version-script=libinchi.map
ifdef ISLINUX
LINUX_FPIC  = -fPIC
LINUX_Z_RELRO = ,-z,relro
endif
endif

# === version ===
MAIN_VERSION = .1
VERSION = $(MAIN_VERSION).04.00

# === executable & library directory ===
ifndef LIB_DIR
  LIB_DIR = result
endif

# === InChI Library name ===
ifndef INCHI_LIB_NAME
  INCHI_LIB_NAME = libinchi
endif
INCHI_LIB_PATHNAME = $(LIB_DIR)/$(INCHI_LIB_NAME)

# === Main program nane ====
ifndef INCHI_MAIN_NAME
  ifdef windir
    EXE = .exe
  else
    EXE =
  endif
  
  INCHI_MAIN_NAME = inchi_main$(EXE)
endif
INCHI_MAIN_PATHNAME = $(LIB_DIR)/$(INCHI_MAIN_NAME)

# === Linker to create (Shared) InChI library ====
ifndef SHARED_LINK
  SHARED_LINK = gcc -shared
endif

# === Linker to create Main program =====
ifndef LINKER
  ifndef windir
  ifdef ISLINUX
     LINKER_CWD_PATH = -Wl,-R,""
  endif
  endif
  LINKER = gcc -s $(LINKER_CWD_PATH)
endif

ifndef P_LIBR
  P_LIBR = ../inchi_dll/
endif

ifndef P_MAIN
  P_MAIN = ../inchi_main/
endif


# === C Compiler ===============
ifndef C_COMPILER
  C_COMPILER = gcc
endif

# === C Compiler Options =======
ifndef C_OPTIONS
  C_OPTIONS = -ansi -O3 -c
  ifndef windir
  ifdef ISLINUX
    ifndef C_SO_OPTIONS
      C_SO_OPTIONS = $(LINUX_FPIC)
    endif
  endif
  endif
  ifndef C_MAIN_OPTIONS
    C_MAIN_OPTIONS = -DBUILD_LINK_AS_DLL
  endif
endif

ifdef windir
ifdef ADD_PASCAL_INCHI_CALLS
  INCHI_DEF        = $(INCHI_LIB_NAME)-pas.def
  SHARED_LINK_PARM = --add-stdcall-alias  
else
  INCHI_DEF        = $(INCHI_LIB_NAME).def
  SHARED_LINK_PARM =
endif
endif

ifdef CREATE_MAIN

INCHI_MAIN_SRCS = $(P_MAIN)e_0dstereo.c   $(P_MAIN)e_ichimain.c   \
                  $(P_MAIN)e_ichi_io.c    $(P_MAIN)e_ichi_parms.c \
                  $(P_MAIN)e_inchi_atom.c $(P_MAIN)e_mol2atom.c   \
                  $(P_MAIN)e_readinch.c   $(P_MAIN)e_readmol.c    \
                  $(P_MAIN)e_readstru.c   $(P_MAIN)e_util.c       \
                  $(P_MAIN)e_ichimain_a.c                      


# === InChI Main object files ============

INCHI_MAIN_OBJS = e_0dstereo.o    e_ichimain.o   \
                  e_ichi_io.o     e_ichi_parms.o \
                  e_inchi_atom.o  e_mol2atom.o   \
                  e_readinch.o    e_readmol.o    \
                  e_readstru.o    e_util.o    \
                  e_ichimain_a.o

# === InChI Main Link rule ================

ifdef windir

$(INCHI_MAIN_PATHNAME) : $(INCHI_MAIN_OBJS) $(INCHI_LIB_PATHNAME).a
	$(LINKER) -o $(INCHI_MAIN_PATHNAME) $(INCHI_MAIN_OBJS) \
  $(INCHI_LIB_PATHNAME).a -lm

else

$(INCHI_MAIN_PATHNAME) : $(INCHI_MAIN_OBJS) $(INCHI_LIB_PATHNAME).so$(VERSION)
	$(LINKER) -o $(INCHI_MAIN_PATHNAME) $(INCHI_MAIN_OBJS) \
  $(INCHI_LIB_PATHNAME).so$(VERSION) -lm

endif

# === InChI Main compile rule ============

%.o: $(P_MAIN)%.c
	$(C_COMPILER) $(C_MAIN_OPTIONS) $(C_OPTIONS) $<

endif


# === InChI Library Source files ============

INCHI_LIB_SRCS = $(P_LIBR)ichican2.c       $(P_LIBR)ichicano.c \
                 $(P_LIBR)ichicans.c       $(P_LIBR)ichiisot.c \
                 $(P_LIBR)ichilnct.c       $(P_LIBR)ichimak2.c \
                 $(P_LIBR)ichimake.c       $(P_LIBR)ichimap1.c \
                 $(P_LIBR)ichimap2.c       $(P_LIBR)ichimap4.c \
                 $(P_LIBR)ichinorm.c       $(P_LIBR)ichiparm.c \
                 $(P_LIBR)ichiprt1.c       $(P_LIBR)ichiprt2.c \
                 $(P_LIBR)ichiprt3.c       $(P_LIBR)ichiqueu.c \
                 $(P_LIBR)ichiring.c       $(P_LIBR)ichisort.c \
                 $(P_LIBR)ichister.c       $(P_LIBR)ichitaut.c \
                 $(P_LIBR)ichi_bns.c       $(P_LIBR)inchi_dll.c \
                 $(P_LIBR)ichiread.c       $(P_LIBR)ichirvr1.c \
                 $(P_LIBR)ichirvr2.c       $(P_LIBR)ichirvr3.c \
                 $(P_LIBR)ichirvr4.c       $(P_LIBR)ichirvr5.c \
                 $(P_LIBR)ichirvr6.c       $(P_LIBR)ichirvr7.c \
                 $(P_LIBR)inchi_dll_main.c $(P_LIBR)runichi.c  \
                 $(P_LIBR)inchi_dll_a.c    $(P_LIBR)inchi_dll_a2.c  \
                 $(P_LIBR)ikey_dll.c       $(P_LIBR)ikey_base26.c  \
                 $(P_LIBR)ichi_io.c  \
                 $(P_LIBR)sha2.c           $(P_LIBR)strutil.c  \
                 $(P_LIBR)util.c

# === InChI Library Object files ============

INCHI_LIB_OBJS = ichican2.o       ichicano.o \
                 ichicans.o       ichiisot.o \
                 ichilnct.o       ichimak2.o \
                 ichimake.o       ichimap1.o \
                 ichimap2.o       ichimap4.o \
                 ichinorm.o       ichiparm.o \
                 ichiprt1.o       ichiprt2.o \
                 ichiprt3.o       ichiqueu.o \
                 ichiring.o       ichisort.o \
                 ichister.o       ichitaut.o \
                 ichi_bns.o       inchi_dll.o \
                 ichiread.o       ichirvr1.o \
                 ichirvr2.o       ichirvr3.o \
                 ichirvr4.o       ichirvr5.o \
                 ichirvr6.o       ichirvr7.o \
                 inchi_dll_main.o runichi.o  \
                 inchi_dll_a.o    inchi_dll_a2.o  \
                 ikey_dll.o       ikey_base26.o  \
                 ichi_io.o  \
                 sha2.o           strutil.o  \
                 util.o

# === InChI Library link rule =========

ifdef windir

$(INCHI_LIB_PATHNAME).a: $(INCHI_LIB_OBJS)
	$(SHARED_LINK) -o $(INCHI_LIB_PATHNAME).dll $(INCHI_DEF) \
  $(INCHI_LIB_OBJS) \
  -Wl,--out-implib,$(INCHI_LIB_PATHNAME).a,-soname,$(INCHI_LIB_PATHNAME).dll

else

$(INCHI_LIB_PATHNAME).so$(VERSION): $(INCHI_LIB_OBJS)
	$(SHARED_LINK) $(SHARED_LINK_PARM) -o \
  $(INCHI_LIB_PATHNAME).so$(VERSION) \
  $(INCHI_LIB_OBJS) \
  -Wl$(LINUX_MAP)$(LINUX_Z_RELRO),-soname,$(INCHI_LIB_NAME).so$(MAIN_VERSION)
	ln -fs $(INCHI_LIB_NAME).so$(VERSION) \
  $(INCHI_LIB_PATHNAME).so$(MAIN_VERSION)

endif

# === InChI Library compile rule =========

%.o: $(P_LIBR)%.c
	$(C_COMPILER) $(C_SO_OPTIONS) $(C_OPTIONS) $<

