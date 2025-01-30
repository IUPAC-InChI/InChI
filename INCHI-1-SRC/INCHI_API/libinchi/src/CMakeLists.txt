target_sources(
    inchi
PRIVATE
    ixa/ixa_inchikey_builder.c
    ixa/ixa_read_mol.c
    ixa/ixa_status.c
    ixa/ixa_builder.c
    ixa/ixa_mol.c
    ixa/ixa_read_inchi.c

    ichilnct.c
    inchi_dll.c
    inchi_dll_main.c
    inchi_dll_a.c
    inchi_dll_a2.c
    inchi_dll_b.c
)