cmake_minimum_required(VERSION 3.0)

project(test_lexbor_html)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_HTML_SOURCES "*.c")

################
## ARGS for tests
#########################
set(lexbor_html_tokenizer_tokens_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/html/tokenizer")
set(lexbor_html_tree_builder_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/html/html5_test")

################
## Create Unit libraries
#########################
EXECUTABLE_LIST("${TEST_LEXBOR_HTML_SOURCES}" ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
APPEND_TESTS("lexbor_html_" "${TEST_LEXBOR_HTML_SOURCES}")
