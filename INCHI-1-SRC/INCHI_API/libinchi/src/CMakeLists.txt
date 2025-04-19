cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(LibInChI_API VERSION 1.7)

add_library(libinchi_compiler_flags INTERFACE)
target_compile_features(libinchi_compiler_flags INTERFACE c_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:C,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:C,MSVC>")
target_compile_options(libinchi_compiler_flags INTERFACE
	"$<${gcc_like_cxx}:$<BUILD_INTERFACE:-g;-O1;-c;-fno-strict-aliasing;-Wno-all>>"
	"$<${msvc_cxx}:$<BUILD_INTERFACE:-W3;-MT;-O2>>"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/static")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(P_BASE "${CMAKE_CURRENT_SOURCE_DIR}/../../../INCHI_BASE/src")
set(P_IXA "${CMAKE_CURRENT_SOURCE_DIR}/ixa")
set(P_CURRENT "${CMAKE_CURRENT_SOURCE_DIR}")
set(P_LIBINCHI_MAP "${CMAKE_CURRENT_SOURCE_DIR}/../gcc")
set(P_VC "${CMAKE_CURRENT_SOURCE_DIR}/../vc14")

include_directories(${P_BASE} ${P_CURRENT} ${P_IXA})

add_library(libinchi SHARED)

target_sources(libinchi PRIVATE
	ichilnct.c
	inchi_dll.c
	inchi_dll.h
	inchi_dll_a.c
	inchi_dll_a.h
	inchi_dll_a2.c
	inchi_dll_b.c
	inchi_dll_b.h
	inchi_dll_main.c
	inchi_dll_main.h
	${P_IXA}/ixa_builder.c
	${P_IXA}/ixa_inchikey_builder.c
	${P_IXA}/ixa_mol.c
	${P_IXA}/ixa_mol.h
	${P_IXA}/ixa_read_inchi.c
	${P_IXA}/ixa_read_mol.c
	${P_IXA}/ixa_status.c
	${P_IXA}/ixa_status.h
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
set_target_properties(libinchi PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

target_link_libraries(libinchi PUBLIC libinchi_compiler_flags)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(libinchi PUBLIC ${MATH_LIBRARY})
endif()

target_compile_definitions(libinchi PRIVATE
	fPIC
	COMPILE_ANSI_ONLY
	TARGET_API_LIB
)

target_include_directories(libinchi PUBLIC "${PROJECT_BINARY_DIR}")

string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set_target_properties(libinchi PROPERTIES SOVERSION 1.07 PREFIX "")


if(NOT WIN32)
	# set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--version-script=${P_LIBINCHI_MAP}/libinchi.map,-z,relro")
	target_link_options(libinchi PRIVATE "LINKER:--version-script=${P_LIBINCHI_MAP}/libinchi.map,-z,relro")
endif()