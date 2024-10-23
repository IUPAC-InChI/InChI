cmake_minimum_required(VERSION 3.18)

project( 
    inchi 
    VERSION 1.07
)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

set( CMAKE_C_STANDARD 11 )

# this works but maybe it's preferable to only export 
# the symbols that are needed to consume the library
set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )

add_subdirectory( INCHI-1-SRC/INCHI_BASE/src )
add_subdirectory( INCHI-1-SRC/INCHI_API/libinchi/src )
add_subdirectory( INCHI-1-SRC/INCHI_API/demos/inchi_main/src )
add_subdirectory( INCHI-1-SRC/INCHI_API/demos/mol2inchi/src )
add_subdirectory( INCHI-1-SRC/INCHI_API/demos/test_ixa/src )
add_subdirectory( INCHI-1-SRC/INCHI_EXE/inchi-1/src )

add_custom_target( run-tests 
    COMMAND pytest INCHI-1-TEST/tests/test_executable --exe-path $<TARGET_FILE:inchi-1> -vv
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)