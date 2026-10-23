#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <filesystem>
#include <vector>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichitime.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichicant.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichimain.h"
}

/* Absolute path to the fixture directory, supplied by CMake so the tests do not
   depend on the working directory they happen to be launched from. The literal
   below is only a fallback for builds that do not define it. */
#ifndef FIXTURES_DIR
#define FIXTURES_DIR "../../../../../INCHI-1-TEST/tests/test_unit/fixtures"
#endif

static char* make_arg(const char* s) {
    size_t len = strlen(s) + 1;
    char* p = (char*)malloc(len);
    memcpy(p, s, len);
    return p;
}

char *read_inchi_from_file(const char *filename) {

    std::ifstream txt_in(filename);
    txt_in.is_open();
    std::string line;
    std::string found_inchi;
    while (std::getline(txt_in, line)) {
        // trim leading/trailing whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(start, end - start + 1);
        if (trimmed.rfind("InChI=", 0) == 0) {
            found_inchi = trimmed;
            break;
        } else if (trimmed.find("InChI=", 0) != std::string::npos) {
            size_t start_pos = trimmed.find("InChI=");
            if(start_pos != 0) {
                found_inchi = trimmed.substr(start_pos);
            } else {
                found_inchi = trimmed;
            }

            break;
        }
    }
    txt_in.close();

    return make_arg(found_inchi.c_str());
}

TEST(test_ichimain, test_CalcAndPrintINCHIAndINCHIKEY) {

    INCHI_IOSTREAM input_stream;

    const char *molblock =
        "enhanc_stereo1									   \n"
        "  ACD/LABS08242216132D                            \n"
        "												   \n"
        "  0  0  0  0  0  0  0  0  0  0999 V3000           \n"
        "M  V30 BEGIN CTAB                                 \n"
        "M  V30 COUNTS 18 17 0 0 1                         \n"
        "M  V30 BEGIN ATOM                                 \n"
        "M  V30 1 C 3424.1946 -1936.7935 0 0               \n"
        "M  V30 2 C 3352.3145 -1895.2935 0 0               \n"
        "M  V30 3 C 3280.4346 -1936.7935 0 0               \n"
        "M  V30 4 C 3208.5542 -1895.2935 0 0               \n"
        "M  V30 5 C 3136.6743 -1936.7935 0 0               \n"
        "M  V30 6 C 3064.7944 -1895.2935 0 0               \n"
        "M  V30 7 Br 3136.6743 -2019.7935 0 0              \n"
        "M  V30 8 Cl 3208.5542 -1812.2935 0 0              \n"
        "M  V30 9 Cl 3280.4346 -2019.7935 0 0              \n"
        "M  V30 10 Cl 3352.3145 -1812.2935 0 0             \n"
        "M  V30 11 Cl 3424.1946 -2019.7935 0 0             \n"
        "M  V30 12 C 3496.075 -1895.2935 0 0               \n"
        "M  V30 13 C 3567.9548 -1936.7942 0 0              \n"
        "M  V30 14 C 3639.835 -1895.2944 0 0               \n"
        "M  V30 15 C 3711.7148 -1936.7942 0 0              \n"
        "M  V30 16 Cl 3639.835 -1812.2944 0 0              \n"
        "M  V30 17 Cl 3567.9548 -2019.7942 0 0             \n"
        "M  V30 18 Cl 3496.075 -1812.2937 0 0              \n"
        "M  V30 END ATOM                                   \n"
        "M  V30 BEGIN BOND                                 \n"
        "M  V30 1 1 1 2                                    \n"
        "M  V30 2 1 1 11 CFG=3                             \n"
        "M  V30 3 1 1 12                                   \n"
        "M  V30 4 1 2 3                                    \n"
        "M  V30 5 1 2 10 CFG=1                             \n"
        "M  V30 6 1 3 4                                    \n"
        "M  V30 7 1 3 9 CFG=1                              \n"
        "M  V30 8 1 4 5                                    \n"
        "M  V30 9 1 4 8 CFG=1                              \n"
        "M  V30 10 1 5 6                                   \n"
        "M  V30 11 1 5 7 CFG=1                             \n"
        "M  V30 12 1 12 13                                 \n"
        "M  V30 13 1 12 18 CFG=3                           \n"
        "M  V30 14 1 13 14                                 \n"
        "M  V30 15 1 13 17 CFG=1                           \n"
        "M  V30 16 1 14 15                                 \n"
        "M  V30 17 1 14 16 CFG=1                           \n"
        "M  V30 END BOND                                   \n"
        "M  V30 BEGIN COLLECTION                           \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 1)                 \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(2 2 3)               \n"
        "M  V30 MDLV30/STEABS ATOMS=(2 4 5)                \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(2 12 13)             \n"
        "M  V30 MDLV30/STEREL2 ATOMS=(1 14)                \n"
        "M  V30 END COLLECTION                             \n"
        "M  V30 END CTAB                                   \n"
        "M  END                                            \n";

    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, molblock);

    INCHI_CLOCK ic = {};
    memset(&ic, 0, sizeof(ic));

    STRUCT_DATA *sd = new STRUCT_DATA;
    // sd->ulStructTime = 0;
    INPUT_PARMS *ip = new INPUT_PARMS;

    memset(ip, 0, sizeof(*ip));
    ip->last_struct_number = 1;
    ip->nInputType = INPUT_MOLFILE;
    ip->bINChIOutputOptions = INCHI_OUT_PLAIN_TEXT;

    char *szTitle;
    INCHI_IOSTREAM *inp_file;
    INCHI_IOSTREAM *plog = new INCHI_IOSTREAM;
    INCHI_IOSTREAM *pout = new INCHI_IOSTREAM;
    INCHI_IOSTREAM *pprb = new INCHI_IOSTREAM;
    ORIG_ATOM_DATA orig_at_data = {};
    long *num_inp = new long(0);

    STRUCT_FPTRS *pStructPtrs = nullptr;

    int nRet = 0;
    int have_err_in_GetOneStructure = 0;
    long num_err = 0;
    int output_error_inchi;

    int bReleaseVersion = bRELEASE_VERSION;
    unsigned long  ulDisplTime = 0;
    unsigned long  ulTotalProcessingTime = 0;

    int argc = 0;
    char *argv[1];
    char *szSdfDataValue = nullptr;

    ReadCommandLineParms(argc, (const char**)argv, ip,
        szSdfDataValue, &ulDisplTime,
        bReleaseVersion, plog);

    const char* inchi_filename = FIXTURES_DIR "/output.txt";
    FILE *file_inchi = nullptr;
    file_inchi = fopen(inchi_filename, "w");
    EXPECT_NE(file_inchi, nullptr);

    inchi_ios_init(pout, INCHI_IOS_TYPE_FILE, file_inchi);
    inchi_ios_init(plog, INCHI_IOS_TYPE_STRING, stdout);
    inchi_ios_init(pprb, INCHI_IOS_TYPE_STRING, nullptr);

    inchi_ios_flush2(plog, stderr);

    int ret = GetTheNextRecordOfInputFile(
        &ic,
        sd,
        ip,
        szTitle,
        &input_stream,
        plog,
        pout,
        pprb,
        &orig_at_data,
        num_inp,
        pStructPtrs,
        &nRet,
        &have_err_in_GetOneStructure,
        &num_err,
        output_error_inchi);

    EXPECT_EQ(ret, DO_NEXT_STEP);
    EXPECT_EQ(orig_at_data.num_inp_atoms, 18);
    EXPECT_EQ(orig_at_data.num_inp_bonds, 17);

    CANON_GLOBALS CG = {};
    PINChI2* pINChI[INCHI_NUM] = {};
    PINChI_Aux2* pINChI_Aux[INCHI_NUM] = {};
    ORIG_ATOM_DATA prep_inp_data = {};
    INCHI_IOS_STRING *strbuf = new INCHI_IOS_STRING;
    inchi_strbuf_init(strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);

    unsigned long pulTotalProcessingTime = 0;
    char* pLF;
    char* pTAB;
    char* ikey;
    int silent;

    set_line_separators(ip->bINChIOutputOptions, &pLF, &pTAB);

    ret = CalcAndPrintINCHIAndINCHIKEY(
        &ic,
        &CG,
        sd,
        ip,
        szTitle,
        pINChI,
        pINChI_Aux,
        &input_stream,
        plog,
        pout,
        pprb,
        &orig_at_data,
        &prep_inp_data,
        num_inp,
        pStructPtrs,
        &nRet,
        have_err_in_GetOneStructure,
        &num_err,
        output_error_inchi,
        strbuf,
        &pulTotalProcessingTime,
        pLF,
        pTAB,
        ikey,
        silent
    );

    EXPECT_EQ(ret, DO_NEXT_STEP);

    if (file_inchi != nullptr) {
        fclose(file_inchi);
    }

    char *found_inchi = read_inchi_from_file(inchi_filename);

    EXPECT_NE(found_inchi, nullptr);

    const char *inchi = "InChI=1S/C10H14BrCl7/c1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h3-10H,1-2H3/t3-,4-,5+,6+,7-,8+,9+,10-/m0/s1";

    EXPECT_STREQ(inchi, found_inchi);

    free(found_inchi);

    remove(inchi_filename);

    FreeAllINChIArrays(pINChI, pINChI_Aux, sd->num_components);

    inchi_strbuf_close(strbuf);
    inchi_ios_free_str(&input_stream);
    FreeOrigAtData(&orig_at_data);
    FreeOrigAtData(&prep_inp_data);
    SetBitFree(&CG);

    for (int i = 0; i < MAX_NUM_PATHS; i++)
    {
        if (ip->path[i])
        {
            inchi_free((void*)ip->path[i]); /*  cast deliberately discards 'const' qualifier */
            ip->path[i] = NULL;
        }
    }

    delete ip;
    delete sd;
    delete plog;
    delete pout;
    delete pprb;
    delete num_inp;
    delete strbuf;

}

TEST(test_ichimain, test_ProcessMultipleInputFiles_2mol_files)
{

    std::vector<std::string> expected_inchis = {
        "InChI=1S/C19H21NO4/c1-2-8-20-9-7-18-15-11-3-4-12(21)16(15)24-17(18)13(22)5-6-19(18,23)14(20)10-11/h2-4,14,17,21,23H,1,5-10H2/t14?,17-,18-,19+/m0/s1",
        "InChI=1S/C8H10N4O2/c1-10-4-9-6-5(10)7(13)12(3)8(14)11(6)2/h4H,1-3H3"
    };

    std::vector<std::string> input_mols = {
        "naloxon.mol",
        "caffeine.mol"
    };

    const char *path_fixtures = FIXTURES_DIR;

    char tmpl[] = FIXTURES_DIR "/inchi_mol_test_XXXXXX";
    char *tmpd = mkdtemp(tmpl);

    EXPECT_NE(tmpd, nullptr);

    std::vector<std::string> dist_paths;
    for (auto cur_filename : input_mols) {
        std::string src_path = std::string(path_fixtures) + "/" + cur_filename;
        std::string dst_path = std::string(tmpd) + "/" + cur_filename;
        std::ifstream src(src_path, std::ios::binary);
        ASSERT_TRUE(src.is_open());
        std::ofstream dst(dst_path, std::ios::binary);
        ASSERT_TRUE(dst.is_open());
        dst << src.rdbuf();
        src.close();
        dst.close();

        dist_paths.push_back(dst_path);
    }

    int argc = input_mols.size() + 2;

    char *inchi_filename = make_arg("test_ichimain");

    std::vector<char*> argv_vec;
    argv_vec.push_back(inchi_filename);
    for (const auto &p : dist_paths) {
        argv_vec.push_back(make_arg(p.c_str()));
    }
    argv_vec.push_back(make_arg("-AMI"));
    char** argv = argv_vec.data();

    int ret = ProcessMultipleInputFiles(argc, argv);

    ASSERT_EQ(ret, 0);

    for (int i = 0; i < expected_inchis.size(); i++) {
        std::string out_txt = dist_paths[i] + ".txt";
        char *inchi = read_inchi_from_file(out_txt.c_str());
        ASSERT_STREQ(inchi, expected_inchis[i].c_str());
        free(inchi);
    }

    for (auto p : dist_paths) {

        std::string out_txt = p + ".txt";
        std::string out_log = p + ".log";
        std::string out_prb = p + ".prb";

        remove(p.c_str());
        remove(out_txt.c_str());
        remove(out_log.c_str());
        remove(out_prb.c_str());
    }
    rmdir(tmpd);

    for (auto p : argv_vec) {
        free(p);
    }
}

TEST(test_ichimain, test_ProcessSingleInputFile_caffeine)
{

    const char *filename_caffeine = "caffeine.mol";

    const char *path_fixtures = FIXTURES_DIR;
    char tmpl[] = FIXTURES_DIR "/inchi_mol_test_XXXXXX";
    char *tmpd = mkdtemp(tmpl);
    ASSERT_NE(tmpd, nullptr);

    std::string src_path = std::string(path_fixtures) + "/" + filename_caffeine;
    std::string dst_path = std::string(tmpd) + "/" + filename_caffeine;
    std::ifstream src(src_path, std::ios::binary);
    ASSERT_TRUE(src.is_open());
    std::ofstream dst(dst_path, std::ios::binary);
    ASSERT_TRUE(dst.is_open());
    dst << src.rdbuf();
    src.close();
    dst.close();

    int argc = 2;
    char *inchi_filename = make_arg("test_ichimain");
    char *input_file = make_arg(dst_path.c_str());
    char* argv[] = { inchi_filename, input_file };

    int result = ProcessSingleInputFile(argc, argv);

    EXPECT_EQ(result, 0);

    std::string out_txt = dst_path + ".txt";
    std::string out_log = dst_path + ".log";
    std::string out_prb = dst_path + ".prb";

    struct stat st;
    ASSERT_EQ(stat(out_txt.c_str(), &st), 0);
    EXPECT_GT(st.st_size, 0);
    ASSERT_EQ(stat(out_log.c_str(), &st), 0);
    EXPECT_GT(st.st_size, 0);
    ASSERT_EQ(stat(out_prb.c_str(), &st), 0);
    EXPECT_EQ(st.st_size, 0);

    char *found_inchi = read_inchi_from_file(out_txt.c_str());
    std::string expected_inchi = "InChI=1S/C8H10N4O2/c1-10-4-9-6-5(10)7(13)12(3)8(14)11(6)2/h4H,1-3H3";

    ASSERT_NE(found_inchi, nullptr);
    EXPECT_EQ(found_inchi, expected_inchi);

    // cleanup
    free(inchi_filename);
    free(input_file);
    free(found_inchi);
    remove(out_txt.c_str());
    remove(out_log.c_str());
    remove(out_prb.c_str());
    remove(dst_path.c_str());
    rmdir(tmpd);
}

TEST(test_ichimain, test_ProcessSingleInputFile_2mols_sdf)
{

    const char *filename_2mols = "test_mols_2.sdf";

    const char *path_fixtures = FIXTURES_DIR;
    char tmpl[] = FIXTURES_DIR "/inchi_mol_test_XXXXXX";
    char *tmpd = mkdtemp(tmpl);
    ASSERT_NE(tmpd, nullptr);

    std::string src_path = std::string(path_fixtures) + "/" + filename_2mols;
    std::string dst_path = std::string(tmpd) + "/" + filename_2mols;
    std::ifstream src(src_path, std::ios::binary);
    ASSERT_TRUE(src.is_open());
    std::ofstream dst(dst_path, std::ios::binary);
    ASSERT_TRUE(dst.is_open());
    dst << src.rdbuf();
    src.close();
    dst.close();

    int argc = 2;
    char *inchi_filename = make_arg("test_ichimain");
    char *input_file = make_arg(dst_path.c_str());
    char* argv[] = { inchi_filename, input_file };

    int result = ProcessSingleInputFile(argc, argv);

    EXPECT_EQ(result, 0);

    std::string out_txt = dst_path + ".txt";
    std::string out_log = dst_path + ".log";
    std::string out_prb = dst_path + ".prb";

    struct stat st;
    ASSERT_EQ(stat(out_txt.c_str(), &st), 0);
    EXPECT_GT(st.st_size, 0);
    ASSERT_EQ(stat(out_log.c_str(), &st), 0);
    EXPECT_GT(st.st_size, 0);
    ASSERT_EQ(stat(out_prb.c_str(), &st), 0);
    EXPECT_EQ(st.st_size, 0);

    std::ifstream txt_in(out_txt);
    ASSERT_TRUE(txt_in.is_open());
    std::string line;
    std::vector<std::string> found_inchis;
    while (std::getline(txt_in, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(start, end - start + 1);
        if (trimmed.rfind("InChI=", 0) == 0) {
            found_inchis.push_back(trimmed);
        }
    }
    txt_in.close();

    std::vector<std::string> expected_inchis = {
        "InChI=1S/C66H75Cl2N9O24/c1-23(2)12-34(71-5)58(88)76-49-51(83)26-7-10-38(32(67)14-26)97-40-16-28-17-41(55(40)101-65-56(54(86)53(85)42(22-78)99-65)100-44-21-66(4,70)57(87)24(3)96-44)98-39-11-8-27(15-33(39)68)52(84)50-63(93)75-48(64(94)95)31-18-29(79)19-37(81)45(31)30-13-25(6-9-36(30)80)46(60(90)77-50)74-61(91)47(28)73-59(89)35(20-43(69)82)72-62(49)92/h6-11,13-19,23-24,34-35,42,44,46-54,56-57,65,71,78-81,83-87H,12,20-22,70H2,1-5H3,(H2,69,82)(H,72,92)(H,73,89)(H,74,91)(H,75,93)(H,76,88)(H,77,90)(H,94,95)/t24-,34+,35-,42?,44-,46+,47+,48-,49+,50-,51+,52+,53+,54-,56+,57+,65-,66-/m0/s1",
        "InChI=1S/C8H10N4O2/c1-10-4-9-6-5(10)7(13)12(3)8(14)11(6)2/h4H,1-3H3"
    };

    ASSERT_EQ(found_inchis.size(), expected_inchis.size());
    for (size_t i = 0; i < expected_inchis.size(); ++i) {
        EXPECT_EQ(found_inchis[i], expected_inchis[i]);
    }

    // cleanup
    free(inchi_filename);
    free(input_file);
    remove(out_txt.c_str());
    remove(out_log.c_str());
    remove(out_prb.c_str());
    remove(dst_path.c_str());
    rmdir(tmpd);
}

TEST(test_ichimain, test_GetTheNextRecordOfInputFile)
{

    INCHI_IOSTREAM input_stream;

    const char *molblock =
        "enhanc_stereo1									  \n"
        "  ACD/LABS08242216132D                            \n"
        "												  \n"
        "  0  0  0  0  0  0  0  0  0  0999 V3000           \n"
        "M  V30 BEGIN CTAB                                 \n"
        "M  V30 COUNTS 18 17 0 0 1                         \n"
        "M  V30 BEGIN ATOM                                 \n"
        "M  V30 1 C 3424.1946 -1936.7935 0 0               \n"
        "M  V30 2 C 3352.3145 -1895.2935 0 0               \n"
        "M  V30 3 C 3280.4346 -1936.7935 0 0               \n"
        "M  V30 4 C 3208.5542 -1895.2935 0 0               \n"
        "M  V30 5 C 3136.6743 -1936.7935 0 0               \n"
        "M  V30 6 C 3064.7944 -1895.2935 0 0               \n"
        "M  V30 7 Br 3136.6743 -2019.7935 0 0              \n"
        "M  V30 8 Cl 3208.5542 -1812.2935 0 0              \n"
        "M  V30 9 Cl 3280.4346 -2019.7935 0 0              \n"
        "M  V30 10 Cl 3352.3145 -1812.2935 0 0             \n"
        "M  V30 11 Cl 3424.1946 -2019.7935 0 0             \n"
        "M  V30 12 C 3496.075 -1895.2935 0 0               \n"
        "M  V30 13 C 3567.9548 -1936.7942 0 0              \n"
        "M  V30 14 C 3639.835 -1895.2944 0 0               \n"
        "M  V30 15 C 3711.7148 -1936.7942 0 0              \n"
        "M  V30 16 Cl 3639.835 -1812.2944 0 0              \n"
        "M  V30 17 Cl 3567.9548 -2019.7942 0 0             \n"
        "M  V30 18 Cl 3496.075 -1812.2937 0 0              \n"
        "M  V30 END ATOM                                   \n"
        "M  V30 BEGIN BOND                                 \n"
        "M  V30 1 1 1 2                                    \n"
        "M  V30 2 1 1 11 CFG=3                             \n"
        "M  V30 3 1 1 12                                   \n"
        "M  V30 4 1 2 3                                    \n"
        "M  V30 5 1 2 10 CFG=1                             \n"
        "M  V30 6 1 3 4                                    \n"
        "M  V30 7 1 3 9 CFG=1                              \n"
        "M  V30 8 1 4 5                                    \n"
        "M  V30 9 1 4 8 CFG=1                              \n"
        "M  V30 10 1 5 6                                   \n"
        "M  V30 11 1 5 7 CFG=1                             \n"
        "M  V30 12 1 12 13                                 \n"
        "M  V30 13 1 12 18 CFG=3                           \n"
        "M  V30 14 1 13 14                                 \n"
        "M  V30 15 1 13 17 CFG=1                           \n"
        "M  V30 16 1 14 15                                 \n"
        "M  V30 17 1 14 16 CFG=1                           \n"
        "M  V30 END BOND                                   \n"
        "M  V30 BEGIN COLLECTION                           \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 1)                 \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(2 2 3)               \n"
        "M  V30 MDLV30/STEABS ATOMS=(2 4 5)                \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(2 12 13)             \n"
        "M  V30 MDLV30/STEREL2 ATOMS=(1 14)                \n"
        "M  V30 END COLLECTION                             \n"
        "M  V30 END CTAB                                   \n"
        "M  END                                            \n";

    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, molblock);

    INCHI_CLOCK ic = {};
    memset(&ic, 0, sizeof(ic));

    STRUCT_DATA *sd = new STRUCT_DATA;
    INPUT_PARMS *ip = new INPUT_PARMS;

    char *szTitle;

    INCHI_IOSTREAM *plog = new INCHI_IOSTREAM;
    INCHI_IOSTREAM *pout = new INCHI_IOSTREAM;
    INCHI_IOSTREAM *pprb = new INCHI_IOSTREAM;
    ORIG_ATOM_DATA orig_at_data = {};
    long *num_inp = new long(0);

    STRUCT_FPTRS *pStructPtrs = nullptr;

    int nRet = 0;
    int have_err_in_GetOneStructure = 0;
    long *num_err;
    int output_error_inchi;

    memset(ip, 0, sizeof(*ip));
    ip->last_struct_number = 1;
    ip->nInputType = INPUT_MOLFILE;

    inchi_ios_init(pout, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_init(plog, INCHI_IOS_TYPE_STRING, stdout);
    inchi_ios_init(pprb, INCHI_IOS_TYPE_STRING, nullptr);

    inchi_ios_flush2(plog, stderr);

    int ret = GetTheNextRecordOfInputFile(
        &ic,
        sd,
        ip,
        szTitle,
        &input_stream,
        plog,
        pout,
        pprb,
        &orig_at_data,
        num_inp,
        pStructPtrs,
        &nRet,
        &have_err_in_GetOneStructure,
        num_err,
        output_error_inchi);

    EXPECT_EQ(ret, DO_NEXT_STEP);

    EXPECT_EQ(orig_at_data.num_inp_atoms, 18);
    EXPECT_EQ(orig_at_data.num_inp_bonds, 17);

    inchi_ios_free_str(&input_stream);
    FreeOrigAtData(&orig_at_data);

    for (int i = 0; i < MAX_NUM_PATHS; i++)
    {
        if (ip->path[i])
        {
            inchi_free((void*)ip->path[i]);
            ip->path[i] = NULL;
        }
    }
    delete ip;
    delete sd;

    delete plog;
    delete pout;
    delete pprb;
    delete num_inp;
}
