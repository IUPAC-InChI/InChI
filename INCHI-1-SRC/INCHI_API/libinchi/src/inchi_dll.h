/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef __INCHI_DLL_H__
#define __INCHI_DLL_H__


#define INCHI_MAX_NUM_ARG 32

int parse_options_string( char *cmd,
                          const char *argv[],
                          int maxargs );
void produce_generation_output( inchi_Output *out,
                                STRUCT_DATA *sd,
                                INPUT_PARMS *ip,
                                INCHI_IOSTREAM *log_file,
                                INCHI_IOSTREAM *out_file );
void copy_corrected_log_tail( inchi_Output *out,
                              INCHI_IOSTREAM *log_file );

int input_erroneously_contains_pseudoatoms( inchi_Input *inp,
                                            inchi_Output *out );

#endif /* __INCHI_DLL_H__ */
