add_executable(
    inchi_main

    e_0dstereo.c
    e_ichimain.c
    e_ichi_io.c
    e_ichi_parms.c
    e_inchi_atom.c
    e_mol2atom.c
    e_readinch.c
    e_readmol.c
    e_readstru.c
    e_util.c
    e_ichimain_a.c

)

target_compile_definitions(
    inchi_main
PRIVATE
    BUILD_LINK_AS_DLL 
    TARGET_EXE_USING_API
)

target_link_libraries(
    inchi_main
    PRIVATE
    inchi
)
