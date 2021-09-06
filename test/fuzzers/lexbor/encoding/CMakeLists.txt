cmake_minimum_required(VERSION 2.8.12)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE FUZZ_LEXBOR_ENCODING_SOURCES "*.c")

################
## Create
#########################
EXECUTABLE_LIST("fuzz_lexbor_encoding_" "${FUZZ_LEXBOR_ENCODING_SOURCES}" ${TEST_LEXBOR_LIB_NAME})
