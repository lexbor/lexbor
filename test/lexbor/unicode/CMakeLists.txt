cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_UNICODE_SOURCES "*.c")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_unicode_" "${TEST_LEXBOR_UNICODE_SOURCES}" ${TEST_DEPS_LIB_NAMES})
APPEND_TESTS("lexbor_unicode_" "${TEST_LEXBOR_UNICODE_SOURCES}")
