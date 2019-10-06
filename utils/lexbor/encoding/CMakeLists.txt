cmake_minimum_required(VERSION 2.8)

project(utils_lexbor_encoding)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB UTILS_LEXBOR_ENCODING_SOURCES "*.c")

################
## Create Utils
#########################
EXECUTABLE_LIST("utils_encoding_" "${UTILS_LEXBOR_ENCODING_SOURCES}" ${LEXBOR_LIB_NAME})
