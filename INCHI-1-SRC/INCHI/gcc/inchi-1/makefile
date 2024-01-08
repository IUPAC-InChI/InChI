# This will create:
# 32-bit executable under 32-bit Windows and 32-bit Linux,
# or 64-bit executable under 64-bit Linux/AMD64
#
# To make 32-bit executable under 64-bit Linux/AMD64, use makefile32
#
#
#
ifndef C_COMPILER
  C_COMPILER   = gcc
endif

ifndef CPP_COMPILER
  CPP_COMPILER = g++
endif

ifndef LINKER
  LINKER = g++ -s
endif

ifndef INCHI_EXECUTABLE
  ifdef windir
    EXE = .exe
  else
    EXE =
  endif
  INCHI_EXECUTABLE = inchi-1$(EXE)
endif

ifndef P_MAIN
  P_MAIN = ../../main
endif
ifndef P_LIBR
  P_LIBR = ../../common
endif

#P_INCL = -I$(P_MAIN) -I$(P_LIBR)
P_INCL = -I$(P_MAIN) -I$(P_LIBR)

C_COMPILER_OPTIONS = $(P_INCL) -ansi -DCOMPILE_ANSI_ONLY -O3 -c
#C_COMPILER_OPTIONS = -c $(P_INCL) -ansi -O3 -fsigned-char -ffunction-sections -fexpensive-optimizations -fstack-check -fexceptions -Wall -pedantic -Wbad-function-cast -Wreturn-type -Wformat -Wuninitialized -Wcast-align -Wshadow -Wunused -Wunused-value -Wunused-variable -Wunused-function -Wunused-parameter -Wunused-label -Wcomment -Wcast-qual -Wconversion -Wimplicit-int -Wmissing-braces -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wsign-compare -Wfloat-equal -Wstrict-prototypes -Wwrite-strings -Wundef -Waggregate-return -Wchar-subscripts -Wformat-nonliteral -Wnested-externs -Wsequence-point -Wpointer-arith -mfancy-math-387 -mieee-fp -mno-soft-float

ifdef windir
# no -ansi option due to reported MinGw bug
  CPP_COMPILER_OPTIONS = $(P_INCL) -D_LIB -O3 -frtti -c
  #CPP_COMPILER_OPTIONS = -c $(P_INCL) -D_LIB -O3 -frtti -Wall -pedantic -Wreturn-type -Wformat -Wuninitialized -ffunction-sections -fexpensive-optimizations -fstack-check -fexceptions -Wcast-align -Wshadow -Wunused -Wunused-value -Wunused-variable -Wunused-function -fsigned-char -Wcast-qual -Wconversion -Wmissing-braces -Wredundant-decls -Wsign-compare -Wfloat-equal -Wwrite-strings -mfancy-math-387 -mieee-fp -mno-soft-float
else
  CPP_COMPILER_OPTIONS = $(P_INCL) -D_LIB -ansi -O3 -frtti -c
  #CPP_COMPILER_OPTIONS = -c $(P_INCL) -D_LIB -ansi -O3 -frtti -Wall -pedantic -Wreturn-type -Wformat -Wuninitialized -ffunction-sections -fexpensive-optimizations -fstack-check -fexceptions -Wcast-align -Wshadow -Wunused -Wunused-value -Wunused-variable -Wunused-function -fsigned-char -Wcast-qual -Wconversion -Wmissing-braces -Wredundant-decls -Wsign-compare -Wfloat-equal -Wwrite-strings -mfancy-math-387 -mieee-fp -mno-soft-float
endif


#LINKER_OPTIONS = -static-libgcc
#LINKER_OPTIONS = -Wall -Wunused -Wunused-function



INCHI_SRCS = $(P_LIBR)/ichican2.c  $(P_LIBR)/ichiprt1.c \
      $(P_LIBR)/ichicano.c             $(P_LIBR)/ichiprt2.c \
      $(P_LIBR)/ichicans.c             $(P_LIBR)/ichiprt3.c \
      $(P_LIBR)/ichiisot.c             $(P_LIBR)/ichiqueu.c \
      $(P_LIBR)/ichiring.c   \
      $(P_LIBR)/ichimak2.c             $(P_LIBR)/ichisort.c \
      $(P_LIBR)/ichimake.c             $(P_LIBR)/ichister.c \
      $(P_LIBR)/ichimap1.c             $(P_LIBR)/ichitaut.c \
      $(P_LIBR)/ichimap2.c             $(P_LIBR)/ichi_bns.c \
      $(P_LIBR)/runichi.c    \
      $(P_LIBR)/ichi_io.c    \
      $(P_LIBR)/ichimap4.c             $(P_LIBR)/strutil.c  \
      $(P_LIBR)/ichinorm.c             $(P_LIBR)/util.c     \
      $(P_LIBR)/ichiparm.c                                  \
      $(P_LIBR)/ichiread.c             $(P_LIBR)/ichirvr1.c \
      $(P_LIBR)/ichirvr2.c             $(P_LIBR)/ichirvr3.c \
      $(P_LIBR)/ichirvr4.c             $(P_LIBR)/ichirvr5.c \
      $(P_LIBR)/ichirvr6.c             $(P_LIBR)/ichirvr7.c \
      $(P_LIBR)/ikey_base26.c          $(P_LIBR)/ikey_dll.c \
      $(P_LIBR)/sha2.c       \
      $(P_MAIN)/dispstru.c             $(P_MAIN)/mol2atom.c \
      $(P_MAIN)/ichialt.c              $(P_MAIN)/readinch.c \
      $(P_MAIN)/ichimain.c             $(P_MAIN)/readmol.c  

INCHI_OBJS = ichican2.o              ichiprt1.o \
             ichicano.o              ichiprt2.o \
             ichicans.o              ichiprt3.o \
             ichiisot.o              ichiqueu.o \
             ichiring.o                         \
             ichimak2.o              ichisort.o \
             ichimake.o              ichister.o \
             ichimap1.o              ichitaut.o \
             ichimap2.o              ichi_bns.o \
             runichi.o  \
             ichi_io.o  \
             ichimap4.o              strutil.o  \
             ichinorm.o              util.o     \
             ichiparm.o                         \
             ichiread.o              ichirvr1.o \
             ichirvr2.o              ichirvr3.o \
             ichirvr4.o              ichirvr5.o \
             ichirvr6.o              ichirvr7.o \
             ikey_base26.o           ikey_dll.o \
             sha2.o    \
             dispstru.o              mol2atom.o \
             readinch.o \
             ichimain.o              readmol.o  


$(INCHI_EXECUTABLE) : $(INCHI_OBJS)
	$(LINKER) $(LINKER_OPTIONS) -o $(INCHI_EXECUTABLE) $(INCHI_OBJS) -lm

%.o: $(P_LIBR)/%.c
	$(C_COMPILER) $(C_COMPILER_OPTIONS) $<

%.o: $(P_MAIN)/%.c
	$(C_COMPILER) $(C_COMPILER_OPTIONS) $<

%.o: $(P_MAIN)/%.cpp
	$(CPP_COMPILER) $(CPP_COMPILER_OPTIONS) $<

