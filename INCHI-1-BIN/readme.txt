/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.03
 * June 15, 2010
 *
 * Originally developed at NIST
 * Modifications and additions by IUPAC and the InChI Trust
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC);
 * you can redistribute this software and/or modify it under the terms of 
 * the GNU Lesser General Public License as published by the Free Software 
 * Foundation:
 * http://www.opensource.org/licenses/lgpl-2.1.php
 */

*****
This is the release of the IUPAC International Chemical
Identifier with InChIKey, version 1, software version 1.03.
(http://www.iupac.org/projects/2000/2000-025-1-800.html
http://www.iupac.org/inchi).

This software supports both standard and non-standard InChI/InChIKey.
*****


This directory includes stand-alone executables for generating the 
InChI/InChIKey, all in a single zip file.

Included is a 'command line' version of InChI software inchi-1.exe.
This Windows 32-bit executable is in the directory win32.

Linux 32-bit and 64-bit versions are located in the files inchi-1.gz 
in the directories linux/32bit and linux/64bit, respectively.

Also included (directory win32) is winchi-1.exe, a conventional Windows 
application that reads structure files from the disk or through the 
Window's clipboard. Processed structures are shown in the upper portion 
of the output screen and text output (including the Identifier itself and 
auxiliary information) is shown in the lower portion of the output screen. 

Options for this program can be seen by starting the program without command 
line options from the 'Command Prompt' ('DOS Window').

By default, InChI software v. 1.03 produces standard InChI.
In particular, the standard identifier is generated when the software is used 
without any specifically added InChI options. If some options are specified, 
and at least one of them qualifies as related to non-standard InChI,
the software produces non-standard Identifier(s). For the details, consult 
InChI User's Guide.


Using winchi-1: entering structures.

Using the Windows clipboard: Molfile formatted structures (connection
tables) that are 'copied' to the clipboard (using Ctrl-C or Edit/Copy
choices), may then be 'pasted' (Ctrl-V or Edit/Paste) in the program.
For ISIS/Draw users this will require the setting 'Copy Mol/Rxnfile
to Clipboard' to be ON (Options/Settings/General tab in V2.x).
ACD/ChemSketch works by default. In general, a structure may be
copied from any application that populates the clipboard with a 
commonly used Molfile-like (MDLCT) structure format.

If problems are encountered, saving structures as Molfile and reading
them in winchi-1.exe is recommended.

Using MOL, SDF, or CML files: A set of files are included for testing.
Each MOL file contains a single structure, while the SDF file
contains many structures. If an SDF or CML file is selected, you can
scroll through the structures with the '>>' and '<<' buttons.

Files may be selected in one of four ways:
1) using the File/Open menu choices - then pressing the '...' button
   on the upper right of the dialog box. 
2) the above dialog box may be opened directly with the 'Open'
   button on the screen.
3) you may 'drag and drop' the input structure files from the 'Windows
   Explorer' program to the winchi-1.exe program window or to its
   icon on the Windows desktop.
4) you may copy and paste a structure from several structure editors.



=========
  FILES 
=========

readme.txt                  This file

win32                       SUB-DIRECTORY
    winchi-1.exe            InChI Windows program
    inchi-1.exe             InChI command line program

linux                       SUB-DIRECTORY

    32bit                       SUB-DIRECTORY
        inchi-1.gz              Linux i-386 executable of inchi-1
                                (needs libstdc++.so.6)

    64bit                       SUB-DIRECTORY
        inchi-1.gz              Linux i-386amd64 executable of inchi-1



InChI discussion group
=======================
https://lists.sourceforge.net/lists/listinfo/inchi-discuss
