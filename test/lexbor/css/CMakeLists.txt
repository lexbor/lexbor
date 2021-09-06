cmake_minimum_required(VERSION 2.8.12)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_CSS_SOURCES "*.c")

################
## ARGS for tests
#########################
set(syntax_tokenizer_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/syntax/tokenizer")
set(syntax_style_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/lexbor.css")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_css_" "${TEST_LEXBOR_CSS_SOURCES}" ${TEST_DEPS_LIB_NAMES})
APPEND_TESTS("lexbor_css_" "${TEST_LEXBOR_CSS_SOURCES}")
