cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

set(P_BASE "${CMAKE_SOURCE_DIR}/INCHI-1-SRC/INCHI_BASE/src")
set(P_CLI "${CMAKE_SOURCE_DIR}/INCHI-1-SRC/INCHI_EXE/inchi-1/src")
file(GLOB BASE_SOURCES CONFIGURE_DEPENDS
     "${P_BASE}/*.c"
     "${P_BASE}/*.h"
     "${P_CLI}/*.c"
     "${P_CLI}/*.h"
)
add_library(test_dependencies STATIC ${BASE_SOURCES})
target_compile_definitions(test_dependencies PUBLIC TARGET_EXE_STANDALONE)
target_compile_options(test_dependencies INTERFACE "-g")

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp")
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name "${test_src}" NAME_WE)
    add_executable(${test_name} ${test_src})
    target_compile_options(${test_name} PRIVATE
        "$<$<CONFIG:Debug>:-g;-O0;-fsanitize=address>"
    )
    target_link_options(${test_name} PRIVATE
        "$<$<CONFIG:Debug>:-g;-O0;-fsanitize=address>"
    )
    target_link_libraries(${test_name} PRIVATE gtest_main gmock_main test_dependencies libinchi)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach()
