add_executable(
    inchi-1

    ichimain.c
    dispstru.c

)

target_compile_definitions(
    inchi-1
PRIVATE
    COMPILE_ANSI_ONLY
    TARGET_EXE_STANDALONE
)

target_link_libraries(
    inchi-1
    PRIVATE
    inchi
)
