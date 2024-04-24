/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.07
 * April 30, 2024
 *
 * MIT License
 *
 * Copyright (c) 2024 IUPAC and InChI Trust
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*
* The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * info@inchi-trust.org
 *
*/


This directory contains inchi-1 source code and executables
(command-line executables produce standard and non-standard InChI/InChIKey).


=========
  FILES 
=========

readme.txt        This file

common            SUB-DIRECTORY
                  Part of the source code common with InChI library

main		      SUB-DIRECTORY
                  The rest of the source code

inchi-1       SUB-DIRECTORY
	gcc               SUB-DIRECTORY
                      Contains gcc makefile to create inchi-1 executable

	vc14              SUB-DIRECTORY
                      Contains Microsoft Visual Studio 2015 project 
                      to create inchi-1.exe
                      

Precompiled inchi-1.exe created with Microsoft Visual Studio 2015 is in INCHI-1-BIN 
section of this distribution.



=========
  LINKS
=========

IUPAC                   http://www.iupac.org/inchi
InChI Trust             http://www.inchi-trust.org                                      
InChI discussion group  https://lists.sourceforge.net/lists/listinfo/inchi-discuss
