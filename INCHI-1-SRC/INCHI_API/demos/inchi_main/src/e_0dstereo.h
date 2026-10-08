/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef __0DSTEREO_H__
#define __0DSTEREO_H__


#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif


    int Clear3D2Dstereo( inchi_Input *pInp );
    int set_0D_stereo_parities( inchi_Input *pInp, int bPointedEdgeStereo );

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif /* __0DSTEREO_H__ */
