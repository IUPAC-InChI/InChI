cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

set(P_BASE "${CMAKE_SOURCE_DIR}/INCHI-1-SRC/INCHI_BASE/src")
file(GLOB BASE_SOURCES CONFIGURE_DEPENDS
     "${P_BASE}/*.c"
     "${P_BASE}/*.h"
)
add_library(test_dependencies STATIC ${BASE_SOURCES})
target_compile_definitions(test_dependencies PUBLIC TARGET_EXE_STANDALONE)
target_compile_options(test_dependencies INTERFACE "-g")

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp")
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name "${test_src}" NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE gtest_main gmock_main test_dependencies)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach()
