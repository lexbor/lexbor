cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_CSS_SOURCES "*.c")

################
## ARGS for tests
#########################
set(syntax_tokenizer_from_file_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/lexbor.css")
set(syntax_tokenizer_print_raw_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/lexbor.css")
set(syntax_structure_parse_file_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/lexbor.css")
set(syntax_simple_colorize_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/lexbor.css")
set(StyleSheet_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/lexbor.css")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_CSS_SOURCES}" ${EXAMPLES_DEPS_LIB_NAMES})

IF(LEXBOR_BUILD_TESTS)
    list(REMOVE_ITEM EXAMPLES_LEXBOR_CSS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/syntax/tokenizer/chunks_stdin.c")
    APPEND_TESTS("examples_lexbor_css_" "${EXAMPLES_LEXBOR_CSS_SOURCES}")
ENDIF()