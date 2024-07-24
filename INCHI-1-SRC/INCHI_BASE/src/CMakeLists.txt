add_library( 
    inchi
SHARED
    ichican2.c
    ichicano.c
    ichi_io.c
    ichierr.c
    ichicans.c
    ichiisot.c
    ichimak2.c
    ichimake.c
    ichimap1.c
    ichimap2.c
    ichimap4.c
    ichinorm.c
    ichiparm.c
    ichiprt1.c
    ichiprt2.c
    ichiprt3.c
    ichiqueu.c
    ichiring.c
    ichisort.c
    ichister.c
    ichitaut.c
    ichi_bns.c
    ichiread.c
    ichirvr1.c
    ichirvr2.c
    ichirvr3.c
    ichirvr4.c
    ichirvr5.c
    ichirvr6.c
    ichirvr7.c
    ikey_dll.c
    ikey_base26.c
    mol_fmt1.c
    mol_fmt2.c
    mol_fmt3.c
    mol2atom.c
    mol_fmt4.c
    readinch.c
    runichi.c
    runichi2.c
    runichi3.c
    runichi4.c
    sha2.c
    strutil.c
    util.c
    bcf_s.c
)

target_link_libraries(
    inchi
PUBLIC
    # link libm in linux
    $<$<BOOL:${LINUX}>:m>
)

# not sure this can stay PRIVATE
target_compile_definitions(
    inchi
PRIVATE
    TARGET_API_LIB
    COMPILE_ANSI_ONLY # why would one want to compile without this?
)

# makefile in INCHI-1-INCHI_API/libinchi/gcc
# applies a libinchi.map file to the linker (line 52 and bejond)
# is that necessary? can we do that with cmake? 
# maybe we can acvhieve the same effect with another method
