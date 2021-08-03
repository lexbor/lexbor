cmake_minimum_required(VERSION 2.8)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_CSS_SOURCES "*.c")

################
## ARGS for tests
#########################
set(syntax_parse_file_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/lexbor.css")
set(syntax_print_raw_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/lexbor.css")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_CSS_SOURCES}" ${LEXBOR_LIB_NAME})

IF(LEXBOR_BUILD_TESTS)
    list(REMOVE_ITEM EXAMPLES_LEXBOR_CSS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/syntax/parse_chunks_stdin.c")
    APPEND_TESTS("examples_lexbor_css_" "${EXAMPLES_LEXBOR_CSS_SOURCES}")
ENDIF()