cmake_minimum_required(VERSION 2.8.12)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_NS_SOURCES "*.c")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_ns_" "${TEST_LEXBOR_NS_SOURCES}" ${TEST_DEPS_LIB_NAMES})
APPEND_TESTS("lexbor_ns_" "${TEST_LEXBOR_NS_SOURCES}")
