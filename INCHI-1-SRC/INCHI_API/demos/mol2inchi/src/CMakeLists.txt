add_executable(
    mol2inchi

    mol2inchi.c	
    moreutil.c
)

target_compile_definitions(
    mol2inchi    
PRIVATE
    BUILD_LINK_AS_DLL 
    TARGET_EXE_USING_API
    )
    
target_link_libraries(
    mol2inchi
PRIVATE
    inchi
)