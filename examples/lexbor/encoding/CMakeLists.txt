cmake_minimum_required(VERSION 2.8.12)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_ENCODING_SOURCES "*.c")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_ENCODING_SOURCES}" ${EXAMPLES_DEPS_LIB_NAMES})
