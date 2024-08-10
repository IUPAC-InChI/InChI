# Comment out the next line to create so/dll only
CREATE_MAIN = 1
# Comment out the next line to create mol2inchi executable (otherwise, inchi_main is created)
#CALLER_IS_MOL2INCHI = 1
# djb-rwth: detecting the OS in Makefile -- no need to specify ISLINUX externally; OS can also be defined in command line: e.g. make OS_ID=1
ifeq ($(OS),Windows_NT)
	OS_ID := 0
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		OS_ID := 1
	endif
	ifeq ($(UNAME_S),Darwin)
		OS_ID := 2
	endif
	ifeq ($(UNAME_S),FreeBSD)
		OS_ID := 3
	endif
# djb-rwth: other OSs should be numbered accordingly
endif
# djb-rwth: checking the presence on GCC and Clang/LLVM
GCC_CHECK := $(shell gcc --version)
CLANG_CHECK := $(shell clang --version)
ifneq ($(GCC_CHECK),)
	GCC_DETECTED := 1
	CCN = 1
endif
ifneq ($(CLANG_CHECK),)
	CLANG_DETECTED := 1
	CCN = 2
endif
ifeq ($(CCN),$(filter $(CCN),1 2))
# djb-rwth: Choosing the compiler if both GCC and Clang/LLVM are detected
ifeq ($(GCC_DETECTED),1)
ifeq ($(CLANG_DETECTED),1)
# djb-rwth: Choose C compiler if both GCC and Clang are detected -- CCN = 1 for GCC, CCN = 2 for Clang/LLVM
CCN = 1
endif
endif
# === C Compiler ===============
ifndef C_COMPILER
ifeq ($(CCN),2)
C_COMPILER = clang
$(info Both GCC and Clang/LLVM detected. Compiling with Clang -- please edit makefile to compile with GCC.)
$(info )
else
C_COMPILER = gcc
$(info Both GCC and Clang/LLVM detected. Compiling with GCC (default) -- please edit makefile to compile with Clang/LLVM.)
$(info )
endif
endif
# Linux fpic option: replace -fPIC with -fpic if the latter works
# Comment out "LINUX_Z_RELRO =" if -z relro is not supported
# These options are needed to avoid the following SELinux message:
# "Error: cannot restore segment prot after reloc: Permission denied"
# In addition, inchi.map restricts set of exported from .so
# functions to those which belong to InChI API
ifneq ($(OS_ID),2)
	LINUX_MAP = ,--version-script=libinchi.map
	LINUX_Z_RELRO = ,-z,relro
endif
ifneq ($(OS_ID),0)
	LINUX_FPIC  = -fPIC
# === version ===
	MAIN_VERSION = .1
	VERSION = $(MAIN_VERSION).07
endif
# === executable & library directory ===
ifndef LIB_DIR
	LIB_DIR = ../../../bin/Linux
endif
# === InChI Library name ===
ifndef INCHI_LIB_NAME
	INCHI_LIB_NAME = libinchi
endif
INCHI_LIB_PATHNAME = $(LIB_DIR)/$(INCHI_LIB_NAME)
# === Main program name ====
ifndef API_CALLER_NAME
	ifndef CALLER_IS_MOL2INCHI 
	API_CALLER_NAME = inchi_main
	else
	API_CALLER_NAME = mol2inchi
	endif
endif
API_CALLER_PATHNAME = $(LIB_DIR)/$(API_CALLER_NAME)
# === Linker to create (Shared) InChI library ====
ifndef SHARED_LINK
	SHARED_LINK = $(C_COMPILER) -shared
endif
# === Linker to create Main program =====
ifndef LINKER
	ifdef OS_ID
# or: ifeq ($(OS_ID),$(filter $(OS_ID),0 1 2 3))
# djb-rwth: avoiding the use of LD_LIBRARY_PATH by adding LIB_DIR to runtime library search path
	LINKER_CWD_PATH = -Wl,-R,"",-rpath=$(LIB_DIR)
	endif
	LINKER = $(C_COMPILER) -s $(LINKER_CWD_PATH)
endif
# djb-rwth: space for shared link parameters SHARED_LINK_PARMS
ifndef P_LIBR
	P_LIBR = ../../../libinchi/src/
endif
ifndef P_LIBR_IXA
	P_LIBR_IXA = ../../../libinchi/src/ixa/
endif
ifndef P_BASE
	P_BASE = ../../../../INCHI_BASE/src/
endif
ifndef P_MAIN
	ifndef CALLER_IS_MOL2INCHI
	P_MAIN = ../src/
	else
	P_MAIN = ../src/
	endif
endif
# === C Compiler Options =======
ifndef C_OPTIONS
	ifndef CALLER_IS_MOL2INCHI
	C_OPTIONS = -std=c11 -ansi -O2 -c -fno-strict-aliasing
	else
	C_OPTIONS = -std=c11 -O2 -c -fno-strict-aliasing
	endif
	ifneq ($(OS_ID),0)
	ifndef C_SO_OPTIONS
		C_SO_OPTIONS = $(LINUX_FPIC) -DTARGET_API_LIB -DCOMPILE_ANSI_ONLY
	endif
	else
	ifndef C_SO_OPTIONS
		C_SO_OPTIONS = -DBUILD_LINK_AS_DLL -DTARGET_API_LIB
	endif
	endif
	ifndef C_MAIN_OPTIONS
# djb-rwth: same as above goes here
	C_MAIN_OPTIONS = -DBUILD_LINK_AS_DLL -DTARGET_EXE_USING_API
	endif
endif
ifdef CREATE_MAIN
ifndef CALLER_IS_MOL2INCHI
API_CALLER_SRCS = $(P_MAIN)e_0dstereo.c	\
$(P_MAIN)e_ichimain.c	\
$(P_MAIN)e_ichi_io.c	\
$(P_MAIN)e_ichi_parms.c	\
$(P_MAIN)e_inchi_atom.c	\
$(P_MAIN)e_mol2atom.c	\
$(P_MAIN)e_readinch.c	\
$(P_MAIN)e_readmol.c	\
$(P_MAIN)e_readstru.c	\
$(P_MAIN)e_util.c	\
$(P_MAIN)e_ichimain_a.c
API_CALLER_OBJS = e_0dstereo.o	\
e_ichimain.o	\
e_ichi_io.o	\
e_ichi_parms.o	\
e_inchi_atom.o	\
e_mol2atom.o	\
e_readinch.o	\
e_readmol.o	\
e_readstru.o	\
e_util.o	\
e_ichimain_a.o
else
API_CALLER_SRCS = $(P_MAIN)mol2inchi.c	\
$(P_MAIN)getcputime.c	\
$(P_MAIN)moreutil.c
API_CALLER_OBJS = mol2inchi.o	\
getcputime.o	\
moreutil.o
endif
# === InChI Main Link rule ================
ifeq ($(OS_ID),0)
# djb-rwth: linking to .dll on Windows
$(API_CALLER_PATHNAME) : $(API_CALLER_OBJS) $(INCHI_LIB_PATHNAME).dll$(VERSION)
	$(LINKER) -o $(API_CALLER_PATHNAME) $(API_CALLER_OBJS) \
$(INCHI_LIB_PATHNAME).dll$(VERSION) -lm
else ifeq ($(OS_ID),2)
# jwm: linking to .dylib on OS X
$(API_CALLER_PATHNAME) : $(API_CALLER_OBJS) $(INCHI_LIB_PATHNAME).so$(VERSION)
	$(LINKER) -o $(API_CALLER_PATHNAME) $(API_CALLER_OBJS) \
$(INCHI_LIB_PATHNAME).dylib$(VERSION) -lm
else
# djb-rwth: linking to .so on Linux
$(API_CALLER_PATHNAME) : $(API_CALLER_OBJS) $(INCHI_LIB_PATHNAME).so$(VERSION)
	$(LINKER) -o $(API_CALLER_PATHNAME) $(API_CALLER_OBJS) \
$(INCHI_LIB_PATHNAME).so$(VERSION) -lm
endif
# === InChI Main compile rule ============
%.o: $(P_MAIN)%.c
	$(C_COMPILER) $(C_MAIN_OPTIONS) $(C_OPTIONS) $<
endif
# === InChI Library Object files ============
INCHI_LIB_OBJS = ichican2.o	\
ichicano.o	\
ichi_io.o	\
ichierr.o	\
ichicans.o	\
ichiisot.o	\
ichilnct.o	\
ichimak2.o	\
ichimake.o	\
ichimap1.o	\
ichimap2.o	\
ichimap4.o	\
ichinorm.o	\
ichiparm.o	\
ichiprt1.o	\
ichiprt2.o	\
ichiprt3.o	\
ichiqueu.o	\
ichiring.o	\
ichisort.o	\
ichister.o	\
ichitaut.o	\
ichi_bns.o	\
inchi_dll.o	\
ichiread.o	\
ichirvr1.o	\
ichirvr2.o	\
ichirvr3.o	\
ichirvr4.o	\
ichirvr5.o	\
ichirvr6.o	\
ichirvr7.o	\
ikey_dll.o	\
ikey_base26.o	\
inchi_dll_main.o	\
inchi_dll_a.o	\
inchi_dll_a2.o	\
inchi_dll_b.o	\
ixa_inchikey_builder.o	\
ixa_read_mol.o	\
ixa_status.o	\
ixa_builder.o	\
ixa_mol.o	\
ixa_read_inchi.o	\
mol_fmt1.o	\
mol_fmt2.o	\
mol_fmt3.o	\
mol2atom.o	\
mol_fmt4.o	\
readinch.o	\
runichi.o	\
runichi2.o	\
runichi3.o	\
runichi4.o	\
sha2.o	\
strutil.o	\
util.o \
bcf_s.o
# === InChI Library link rule =========
# === InChI Library link rule =========
ifeq ($(OS_ID),0)
# djb-rwth: creating .dll on Windows
$(INCHI_LIB_PATHNAME).dll$(VERSION): $(INCHI_LIB_OBJS)
	$(SHARED_LINK) $(SHARED_LINK_PARM) -o $(INCHI_LIB_PATHNAME).dll$(VERSION)	\
$(INCHI_LIB_OBJS) -Wl$(LINUX_MAP),-soname,$(INCHI_LIB_NAME).dll$(VERSION) -Wl,--subsystem,windows -lm
else ifeq ($(OS_ID), 2)
# jwm: creating .dylib on OS X
$(INCHI_LIB_PATHNAME).dylib$(VERSION): $(INCHI_LIB_OBJS)
	$(SHARED_LINK) $(SHARED_LINK_PARM) -o $(INCHI_LIB_PATHNAME).dylib$(VERSION)	\
$(INCHI_LIB_OBJS) -Wl$(LINUX_MAP)$(LINUX_Z_RELRO) -install_name $(INCHI_LIB_NAME).dylib$(VERSION) -lm
else
# djb-rwth: creating .so on Linux
$(INCHI_LIB_PATHNAME).so$(VERSION): $(INCHI_LIB_OBJS)
	$(SHARED_LINK) $(SHARED_LINK_PARM) -o $(INCHI_LIB_PATHNAME).so$(VERSION)	\
$(INCHI_LIB_OBJS) -Wl$(LINUX_MAP)$(LINUX_Z_RELRO),-soname,$(INCHI_LIB_NAME).so$(VERSION) -lm
endif
# djb-rwth: no forceful linking is required if .so version extensions are the same
#	ln -fs $(INCHI_LIB_NAME).so$(VERSION)	\
$(INCHI_LIB_PATHNAME).so$(MAIN_VERSION)
# === InChI Library compile rule =========
%.o: $(P_LIBR)%.c
	$(C_COMPILER) $(C_SO_OPTIONS) $(C_OPTIONS) $<
%.o: $(P_LIBR_IXA)%.c
	$(C_COMPILER) $(C_SO_OPTIONS) $(C_OPTIONS) $<
%.o: $(P_BASE)%.c
	$(C_COMPILER) $(C_SO_OPTIONS) $(C_OPTIONS) $<
ifeq ($(CCN),2)
ifneq ($(OS_ID),0)
$(info Please follow the instructions in README.md file before using "inchi_main" ELF file.)
$(info )
endif
endif
else
$(info GCC or Clang not detected. Please edit makefile in order to include available C compiler. Terminating.)
$(info )
endif