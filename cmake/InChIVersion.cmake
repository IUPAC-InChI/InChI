if(NOT DEFINED INCHI_VERSION_FULL)
    set(INCHI_VERSION_FULL "2026.2")
endif()
set(INCHI_VERSION_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

if(NOT INCHI_VERSION_FULL MATCHES "^[0-9][0-9][0-9][0-9]\\.(0|[1-9][0-9]*)(-[a-z0-9]+(_[a-z0-9]+)*)?$")
    message(FATAL_ERROR
        "INCHI_VERSION_FULL must match YYYY.MINOR[-lowercase_modifier], got '${INCHI_VERSION_FULL}'"
    )
endif()

string(REGEX MATCH "^([0-9][0-9][0-9][0-9])\\.([0-9]+)" INCHI_VERSION_MATCH "${INCHI_VERSION_FULL}")
set(INCHI_VERSION_YEAR "${CMAKE_MATCH_1}")
set(INCHI_VERSION_MINOR "${CMAKE_MATCH_2}")
set(INCHI_SOVERSION "${INCHI_VERSION_YEAR}")

function(inchi_configure_version_header output_directory)
    set(INCHI_VERSION_HEADER "${output_directory}/inchi_version.h")
    configure_file(
        "${INCHI_VERSION_MODULE_DIR}/inchi_version.h.in"
        "${INCHI_VERSION_HEADER}"
        @ONLY
    )
endfunction()
