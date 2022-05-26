cmake_minimum_required(VERSION 2.8.12)

IF(CMAKE_VERSION VERSION_LESS "3.0")
    PROJECT("utils_lexbor_encoding")
ELSE()
    cmake_policy(SET CMP0048 NEW)
    PROJECT("utils_lexbor_encoding" VERSION "${LEXBOR_VERSION_STRING}")
ENDIF()

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
