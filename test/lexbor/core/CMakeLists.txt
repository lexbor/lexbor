cmake_minimum_required(VERSION 2.8.12)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_CORE_SOURCES "*.c")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_core_" "${TEST_LEXBOR_CORE_SOURCES}" ${TEST_DEPS_LIB_NAMES})
APPEND_TESTS("lexbor_core_" "${TEST_LEXBOR_CORE_SOURCES}")