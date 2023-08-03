cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_URL_SOURCES "*.c")

################
## ARGS for tests
#########################
set(parser_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/url")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_url_" "${TEST_LEXBOR_URL_SOURCES}" ${TEST_DEPS_LIB_NAMES})
APPEND_TESTS("lexbor_url_" "${TEST_LEXBOR_URL_SOURCES}")
