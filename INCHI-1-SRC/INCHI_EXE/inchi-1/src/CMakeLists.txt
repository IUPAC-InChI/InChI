set(INCHI_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../INCHI_BASE/src)

add_executable(
    inchi-1

    ichimain.c
    dispstru.c

    # sources from library
    # this should replaced by linking to the library, but right now it is not possible
    # because the executable doesn't work unless sources are built with TARGET_EXE_STANDALONE
    ${INCHI_SRC_DIR}/ichi_bns.c
    ${INCHI_SRC_DIR}/ichi_io.c
    ${INCHI_SRC_DIR}/ichicano.c
    ${INCHI_SRC_DIR}/ichican2.c
    ${INCHI_SRC_DIR}/ichicans.c
    ${INCHI_SRC_DIR}/ichierr.c
    ${INCHI_SRC_DIR}/ichiisot.c
    ${INCHI_SRC_DIR}/ichimake.c
    ${INCHI_SRC_DIR}/ichimak2.c
    ${INCHI_SRC_DIR}/ichimap1.c
    ${INCHI_SRC_DIR}/ichimap2.c
    ${INCHI_SRC_DIR}/ichimap4.c
    ${INCHI_SRC_DIR}/ichinorm.c
    ${INCHI_SRC_DIR}/ichiprt1.c
    ${INCHI_SRC_DIR}/ichiprt2.c
    ${INCHI_SRC_DIR}/ichiprt3.c
    ${INCHI_SRC_DIR}/ichiqueu.c
    ${INCHI_SRC_DIR}/ichiring.c
    ${INCHI_SRC_DIR}/ichisort.c
    ${INCHI_SRC_DIR}/ichister.c
    ${INCHI_SRC_DIR}/ichitaut.c
    ${INCHI_SRC_DIR}/ichiparm.c
    ${INCHI_SRC_DIR}/ichiread.c
    ${INCHI_SRC_DIR}/ichirvr1.c
    ${INCHI_SRC_DIR}/ichirvr2.c
    ${INCHI_SRC_DIR}/ichirvr3.c
    ${INCHI_SRC_DIR}/ichirvr4.c
    ${INCHI_SRC_DIR}/ichirvr5.c
    ${INCHI_SRC_DIR}/ichirvr6.c
    ${INCHI_SRC_DIR}/ichirvr7.c
    ${INCHI_SRC_DIR}/ikey_base26.c
    ${INCHI_SRC_DIR}/ikey_dll.c
    ${INCHI_SRC_DIR}/mol_fmt1.c
    ${INCHI_SRC_DIR}/mol_fmt2.c
    ${INCHI_SRC_DIR}/mol_fmt3.c
    ${INCHI_SRC_DIR}/mol_fmt4.c
    ${INCHI_SRC_DIR}/mol2atom.c
    ${INCHI_SRC_DIR}/readinch.c
    ${INCHI_SRC_DIR}/runichi.c
    ${INCHI_SRC_DIR}/runichi2.c
    ${INCHI_SRC_DIR}/runichi3.c
    ${INCHI_SRC_DIR}/runichi4.c
    ${INCHI_SRC_DIR}/sha2.c
    ${INCHI_SRC_DIR}/strutil.c
    ${INCHI_SRC_DIR}/util.c
    ${INCHI_SRC_DIR}/bcf_s.c
)

target_compile_definitions(
    inchi-1
PRIVATE
    COMPILE_ANSI_ONLY
    TARGET_EXE_STANDALONE
)

target_link_libraries(
    inchi-1
PUBLIC
    # link libm in linux
    $<$<BOOL:${LINUX}>:m>
)
