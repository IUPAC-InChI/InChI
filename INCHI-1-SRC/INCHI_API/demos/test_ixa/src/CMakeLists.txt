add_executable(
    test_ixa

    moreutil.c	
    test_ixa.c
)
target_compile_definitions(
    test_ixa
PRIVATE
    BUILD_LINK_AS_DLL 
    TARGET_EXE_USING_API
)

target_link_libraries(
    test_ixa
PRIVATE
    inchi
)