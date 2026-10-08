/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef __READSTRU_H__
#define __READSTRU_H__


#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif



    int e_ReadStructure( STRUCT_DATA *sd, 
                         INPUT_PARMS *ip,
                         INCHI_IOSTREAM *inp_file,
                         INCHI_IOSTREAM *log_file, 
                         INCHI_IOSTREAM *output_file, 
                         INCHI_IOSTREAM *prb_file,
                         inchi_InputEx *pInp, 
                         long num_inp,  
                         int inp_index, 
                         int *out_index );


#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* __READSTRU_H__ */
