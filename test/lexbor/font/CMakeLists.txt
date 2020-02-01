cmake_minimum_required(VERSION 2.8)

project(test_lexbor_font)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_FONT_SOURCES "*.c")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_font_" "${TEST_LEXBOR_FONT_SOURCES}" ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
APPEND_TESTS("lexbor_font_" "${TEST_LEXBOR_FONT_SOURCES}")
