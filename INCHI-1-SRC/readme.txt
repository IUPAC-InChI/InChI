This directory contains InChI Software source codes.
It also contains examples of InChI API usage, for C 
('inchi_main', 'mol2inchi', 'test_ixa'); see projects 
for MS Visual Studio in 'vc14' and for gcc/Linux 
in 'gcc' subdirs) and Python 3 ('python_sample'). 

Also supplied are InChI API Library source codes and 
related projects/makefiles.

For more details, please refer to respective sub-directories.

The general layout is as follows.


INCHI-1-SRC/INCHI_BASE
    src                     C source files of common codebase
                            used by both InChI Library and
                            inchi-1 executable

INCHI-1-SRC/INCHI_EXE
    inchi-1/src             C source files specific for 
                            inchi-1 executable

INCHI-1-SRC/INCHI_API               
    libinchi/src            C source files specific for 
                            InChI Software Library (API)

    demos/inchi_main/src    C source files specific for 
                            inchi_main demo

    demos/mol2inchi/src     C source files specific for 
                            mol2inchi  demo

    demos/test_ixa/src     C source files specific for 
                            test_ixa demo

    demos/python_sample    Python 3 source files specific for 
                            Python demo
