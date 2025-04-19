cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(InChI_CLI VERSION 1.7)

add_library(inchi_compiler_flags INTERFACE)
target_compile_features(inchi_compiler_flags INTERFACE c_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:C,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:C,MSVC>")
target_compile_options(inchi_compiler_flags INTERFACE
	"$<${gcc_like_cxx}:$<BUILD_INTERFACE:-g;-O1;-c;-fno-strict-aliasing;-Wno-all>>"
	"$<${msvc_cxx}:$<BUILD_INTERFACE:-W3;-MT;-O2>>"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/static")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(P_BASE "${CMAKE_CURRENT_SOURCE_DIR}/../../../INCHI_BASE/src")
set(P_VC "${CMAKE_CURRENT_SOURCE_DIR}/../vc14")
set(P_CURRENT "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${P_BASE} ${P_CURRENT})

add_executable(inchi-1)

target_sources(inchi-1 PRIVATE
	dispstru.c
	dispstru.h
	ichimain.c
	${P_BASE}/bcf_s.h
	${P_BASE}/bcf_s.c
	${P_BASE}/extr_ct.h
	${P_BASE}/ichi.h
	${P_BASE}/ichi_bns.c
	${P_BASE}/ichi_bns.h
	${P_BASE}/ichi_io.c
	${P_BASE}/ichi_io.h
	${P_BASE}/ichican2.c
	${P_BASE}/ichicano.c
	${P_BASE}/ichicano.h
	${P_BASE}/ichicans.c
	${P_BASE}/ichicant.h
	${P_BASE}/ichicomn.h
	${P_BASE}/ichicomp.h
	${P_BASE}/ichidrp.h
	${P_BASE}/ichierr.c
	${P_BASE}/ichierr.h
	${P_BASE}/ichiisot.c
	${P_BASE}/ichimain.h
	${P_BASE}/ichimak2.c
	${P_BASE}/ichimake.c
	${P_BASE}/ichimake.h
	${P_BASE}/ichimap1.c
	${P_BASE}/ichimap2.c
	${P_BASE}/ichimap4.c
	${P_BASE}/ichinorm.c
	${P_BASE}/ichinorm.h
	${P_BASE}/ichiparm.c
	${P_BASE}/ichiprt1.c
	${P_BASE}/ichiprt2.c
	${P_BASE}/ichiprt3.c
	${P_BASE}/ichiqueu.c
	${P_BASE}/ichiread.c
	${P_BASE}/ichiring.c
	${P_BASE}/ichiring.h
	${P_BASE}/ichirvr1.c
	${P_BASE}/ichirvr2.c
	${P_BASE}/ichirvr3.c
	${P_BASE}/ichirvr4.c
	${P_BASE}/ichirvr5.c
	${P_BASE}/ichirvr6.c
	${P_BASE}/ichirvr7.c
	${P_BASE}/ichirvrs.h
	${P_BASE}/ichisize.h
	${P_BASE}/ichisort.c
	${P_BASE}/ichister.c
	${P_BASE}/ichister.h
	${P_BASE}/ichitaut.c
	${P_BASE}/ichitaut.h
	${P_BASE}/ichitime.h
	${P_BASE}/ikey_base26.c
	${P_BASE}/ikey_base26.h
	${P_BASE}/ikey_dll.c
	${P_BASE}/inchi_api.h
	${P_BASE}/inchi_gui.c
	${P_BASE}/inchi_gui.h
	${P_BASE}/inchicmp.h
	${P_BASE}/incomdef.h
	${P_BASE}/inpdef.h
	${P_BASE}/ixa.h
	${P_BASE}/logging.h
	${P_BASE}/mode.h
	${P_BASE}/mol_fmt.h
	${P_BASE}/mol_fmt1.c
	${P_BASE}/mol_fmt2.c
	${P_BASE}/mol_fmt3.c
	${P_BASE}/mol_fmt4.c
	${P_BASE}/mol2atom.c
	${P_BASE}/readinch.c
	${P_BASE}/readinch.h
	${P_BASE}/runichi.c
	${P_BASE}/runichi2.c
	${P_BASE}/runichi3.c
	${P_BASE}/runichi4.c
	${P_BASE}/sha2.c
	${P_BASE}/sha2.h
	${P_BASE}/strutil.c
	${P_BASE}/strutil.h
	${P_BASE}/util.c
	${P_BASE}/util.h
	${P_VC}/resource.h
)

set(CMAKE_DEBUG_POSTFIX d)
set_target_properties(inchi-1 PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

target_link_libraries(inchi-1 PUBLIC inchi_compiler_flags)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(inchi-1 PUBLIC ${MATH_LIBRARY})
endif()

target_compile_definitions(inchi-1 PRIVATE
	COMPILE_ANSI_ONLY
	TARGET_EXE_STANDALONE
	ADD_AMI_MODE
)

target_include_directories(inchi-1 PUBLIC "${PROJECT_BINARY_DIR}")

string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

