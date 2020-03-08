cmake_minimum_required(VERSION 2.8)

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
EXECUTABLE_LIST("lexbor_ns_" "${TEST_LEXBOR_NS_SOURCES}" ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
APPEND_TESTS("lexbor_ns_" "${TEST_LEXBOR_NS_SOURCES}")
