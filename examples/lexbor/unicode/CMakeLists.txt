cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_UNICODE_SOURCES "*.c")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_UNICODE_SOURCES}" ${EXAMPLES_DEPS_LIB_NAMES})

IF(LEXBOR_BUILD_TESTS)
    list(REMOVE_ITEM EXAMPLES_LEXBOR_UNICODE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/idna_to_ascii.c")
    list(REMOVE_ITEM EXAMPLES_LEXBOR_UNICODE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/normalization_form_stdin.c")
    APPEND_TESTS("examples_lexbor_unicode_" "${EXAMPLES_LEXBOR_UNICODE_SOURCES}")
ENDIF()