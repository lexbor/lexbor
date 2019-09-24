cmake_minimum_required(VERSION 2.8)

project(examples_lexbor_encoding)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_ENCODING_SOURCES "*.c")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_ENCODING_SOURCES}" ${LEXBOR_LIB_NAME})
