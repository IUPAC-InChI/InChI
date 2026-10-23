#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <filesystem>
#include <vector>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichiprt1.c"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/strutil.c"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichimake.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichicano.h"
}


TEST(test_ichiprt1_enhancedStereo, test_OutputINCHI_StereoLayer_enhanced_stereo_1)
{

    CANON_GLOBALS cg = {0};
    INCHI_IOSTREAM out_file = {0};
    INCHI_IOS_STRING strbuf = {0};
    INCHI_OUT_CTL io = {0};
    ORIG_ATOM_DATA *oad;

    oad = (ORIG_ATOM_DATA *)inchi_calloc(1, sizeof(ORIG_ATOM_DATA));
    oad->v3000 = (OAD_V3000 *)inchi_calloc(1, sizeof(OAD_V3000));

    oad->v3000->n_collections = 5;
    oad->v3000->n_steabs = 1;

    oad->v3000->lists_steabs = (int **)inchi_calloc(oad->v3000->n_steabs, sizeof(int*));
    oad->v3000->lists_steabs[0] = (int *)inchi_calloc(2+2, sizeof(int));

    // STEABS ATOMS=(2 4 5)
    oad->v3000->lists_steabs[0][0] = 1; // - not used
    oad->v3000->lists_steabs[0][1] = 2; // number of members in collection
    oad->v3000->lists_steabs[0][2] = 4; //  member atom numbers
    oad->v3000->lists_steabs[0][3] = 5; //  member atom numbers

    // STERAC2 ATOMS=(1 1)
    oad->v3000->n_sterac = 2;

    oad->v3000->lists_sterac = (int **)inchi_calloc(oad->v3000->n_sterac, sizeof(int*));
    oad->v3000->lists_sterac[0] = (int *)inchi_calloc(2 + 1, sizeof(int));
    oad->v3000->lists_sterac[1] = (int *)inchi_calloc(2 + 2, sizeof(int));

    oad->v3000->lists_sterac[0][0] = 2; // n from "STERACn" tag
    oad->v3000->lists_sterac[0][1] = 1; // number of members in collection
    oad->v3000->lists_sterac[0][2] = 1; // member atom numbers

    // STERAC1 ATOMS=(2 2 3)
    oad->v3000->lists_sterac[1][0] = 1; // STERAC1 ATOMS=(2 2 3)
    oad->v3000->lists_sterac[1][1] = 2; // number of members in collection
    oad->v3000->lists_sterac[1][2] = 2; // member atom numbers
    oad->v3000->lists_sterac[1][3] = 3; // member atom numbers

    oad->v3000->n_sterel = 2;
    oad->v3000->lists_sterel = (int **)inchi_calloc(oad->v3000->n_sterel, sizeof(int*));
    oad->v3000->lists_sterel[0] = (int *)inchi_calloc(2 + 2, sizeof(int));
    oad->v3000->lists_sterel[1] = (int *)inchi_calloc(2 + 1, sizeof(int));

    // STEREL1 ATOMS=(2 12 13)
    oad->v3000->lists_sterel[0][0] = 1;  // n from "STERELn" tag
    oad->v3000->lists_sterel[0][1] = 2;  // number of members in collection
    oad->v3000->lists_sterel[0][2] = 12; // member atom numbers
    oad->v3000->lists_sterel[0][3] = 13; // member atom numbers

    // STEREL2 ATOMS=(1 14)
    oad->v3000->lists_sterel[1][0] = 2;  // n from "STERELn" tag
    oad->v3000->lists_sterel[1][1] = 1;  // number of members in collection
    oad->v3000->lists_sterel[1][2] = 14; // member atom numbers

    INCHI_SORT *inchi_sort = (INCHI_SORT*)inchi_calloc(1, sizeof(INCHI_SORT));

    int num_at = 8;
    int num_iso_at = 1;
    int alloc_mode = 0;
    int bOrigatomflag = 0;
    int found_num_bonds = 0;
    int found_num_isotopic = 0;

    inp_ATOM *atoms = CreateInpAtom(num_at);
    INChI *inchi = Alloc_INChI(atoms, num_at, &found_num_bonds, &found_num_isotopic, 0);
    inchi->nNumberOfAtoms = num_at;

    // every canonical atom is a stereocentre: only those reach /s
    for (int i = 0; i < num_at; i++)
    {
        inchi->Stereo->nNumber[i] = (AT_NUMB)(i + 1);
    }
    inchi->Stereo->nNumberOfStereoCenters = num_at;

    INChI_Aux *pAux = Alloc_INChI_Aux(num_at, num_iso_at, alloc_mode, bOrigatomflag);

    pAux->nNumberOfAtoms = num_at;
    // pAux->nOrigAtNosInCanonOrd = (AT_NUMB*)inchi_calloc(num_at, sizeof(AT_NUMB));

    // canonical atom i+1 maps to original atom i+1
    pAux->nOrigAtNosInCanonOrd[0] = 4;
    pAux->nOrigAtNosInCanonOrd[1] = 5;
    pAux->nOrigAtNosInCanonOrd[2] = 1;
    pAux->nOrigAtNosInCanonOrd[3] = 2;
    pAux->nOrigAtNosInCanonOrd[4] = 3;
    pAux->nOrigAtNosInCanonOrd[5] = 12;
    pAux->nOrigAtNosInCanonOrd[6] = 13;
    pAux->nOrigAtNosInCanonOrd[7] = 14;

    inchi_sort->pINChI[0] = inchi;
    inchi_sort->pINChI_Aux[0] = pAux;
    io.pINChISort = inchi_sort;

    io.bOutType = OUT_TN;
    io.num_components = 1;
    io.sDifSegs[0][DIFS_s_STYPE] = DIFV_OUTPUT_FILL_T;
    io.nCurINChISegment = 0;
    io.iCurTautMode = 0;
    io.bPlainTextTags = 1;
    io.bRelativeStereo[0] = 0;
    io.bRacemicStereo[0] = 0;
    io.bAlways = 0;
    io.nTag = 2; // plain text
    io.bTag1 = IL_STER;
    io.bTag2 = IL_STER | IL_TYPS;

    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);
    inchi_ios_init(&out_file, INCHI_IOS_TYPE_STRING, nullptr);

    int ret = OutputINCHI_StereoLayer_EnhancedStereo(&cg, &out_file, &strbuf, &io, oad, (char*)"", (char*)"");

    // EXPECT_EQ(std::string(strbuf.pStr), "/s1(4,5)2(12,13;14)3(1;2,3)");
    EXPECT_EQ(std::string(strbuf.pStr), "/s1(1,2)2(6,7)(8)3(3)(4,5)");
    EXPECT_EQ(ret, 0);

    for (int i = 0; i < oad->v3000->n_steabs; i++) {
        inchi_free(oad->v3000->lists_steabs[i]);
    }
    inchi_free(oad->v3000->lists_steabs);

    for (int i = 0; i < oad->v3000->n_sterel; i++) {
        inchi_free(oad->v3000->lists_sterel[i]);
    }
    inchi_free(oad->v3000->lists_sterel);

    for (int i = 0; i < oad->v3000->n_sterac; i++) {
        inchi_free(oad->v3000->lists_sterac[i]);
    }
    inchi_free(oad->v3000->lists_sterac);

    inchi_free(oad->v3000);
    inchi_free(oad);

    inchi_free(inchi_sort);

    FreeInpAtom(&atoms);
    Free_INChI_Aux(&pAux);
    Free_INChI(&inchi);

    inchi_strbuf_close(&strbuf);
    inchi_ios_close(&out_file);
}

