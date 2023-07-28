cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE FUZZ_LEXBOR_PUNYCODE_SOURCES "*.c")

################
## Create
#########################
EXECUTABLE_LIST("fuzz_lexbor_punycode_" "${FUZZ_LEXBOR_PUNYCODE_SOURCES}" ${TEST_LEXBOR_LIB_NAME})
