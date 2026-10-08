cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/InChIVersion.cmake)

project(InChI VERSION ${INCHI_VERSION_FULL} LANGUAGES C CXX)

option(COVERAGE "Enable coverage reporting" OFF)

if(COVERAGE)
    message(STATUS "Building with code coverage enabled")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    else()
        message(WARNING "Coverage only supported with GCC or Clang")
    endif()
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(CTest)
include(GoogleTest)
enable_testing()

add_subdirectory(INCHI-1-SRC/INCHI_EXE/inchi-1/src)
add_subdirectory(INCHI-1-SRC/INCHI_API/libinchi/src)
add_subdirectory(INCHI-1-TEST/tests/test_unit)

if(COVERAGE)
    add_custom_target(coverage
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND find . -name '*.gcno' -exec gcov -b -c {} +
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests and generating .gcov coverage reports"
    )
endif()
