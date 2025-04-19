cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(TestIXA_API VERSION 1.7)

add_library(testixa_compiler_flags INTERFACE)
target_compile_features(testixa_compiler_flags INTERFACE c_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:C,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:C,MSVC>")
target_compile_options(testixa_compiler_flags INTERFACE
	"$<${gcc_like_cxx}:$<BUILD_INTERFACE:-g;-O1;-c;-fno-strict-aliasing;-Wno-all>>"
	"$<${msvc_cxx}:$<BUILD_INTERFACE:-W3;-MT;-O2>>"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/static")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(P_BASE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../INCHI_BASE/src")
set(P_LIBINCHI "${CMAKE_CURRENT_SOURCE_DIR}/../../../libinchi/src")
set(P_LIBINCHI_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/../../../libinchi/src/build")
set(P_CURRENT "${CMAKE_CURRENT_SOURCE_DIR}")
set(P_VC "${CMAKE_CURRENT_SOURCE_DIR}/../vc14")

include_directories(${P_CURRENT})

add_subdirectory(${P_LIBINCHI} ${P_LIBINCHI_BUILD})

add_executable(test_ixa)

target_sources(test_ixa PRIVATE
	moreitil.h
	test_ixa.h
	moreutil.c
	test_ixa.c
	${P_BASE}/bcf_s.h
	${P_BASE}/bcf_s.c
	${P_BASE}/logging.h
	${P_VC}/resource.h
)

target_link_libraries(test_ixa PUBLIC libinchi)

set(CMAKE_DEBUG_POSTFIX d)
set_target_properties(test_ixa PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

target_link_libraries(test_ixa PUBLIC testixa_compiler_flags)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(test_ixa PUBLIC ${MATH_LIBRARY})
endif()

target_compile_definitions(test_ixa PRIVATE
	BUILD_LINK_AS_DLL
	TARGET_EXE_USING_API
)

target_include_directories(test_ixa PUBLIC "${PROJECT_BINARY_DIR}")

string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

if(WIN32)
	add_custom_command(TARGET test_ixa POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:test_ixa> $<TARGET_RUNTIME_DLLS:test_ixa>
		COMMAND_EXPAND_LISTS
	)
else()
	add_custom_command(TARGET test_ixa POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libinchi> $<TARGET_FILE_DIR:test_ixa>
		COMMAND_EXPAND_LISTS
	)
endif()
