cmake_minimum_required(VERSION 2.8.12...3.27)

IF(CMAKE_VERSION VERSION_LESS "3.0")
    IF(LEXBOR_BUILD_TESTS_CPP)
        PROJECT("utils_lexbor_encoding")
    ELSE()
        PROJECT("utils_lexbor_encoding" "C")
    ENDIF()
ELSE()
    cmake_policy(SET CMP0048 NEW)

    IF(LEXBOR_BUILD_TESTS_CPP)
        PROJECT("utils_lexbor_encoding" VERSION "${LEXBOR_VERSION_STRING}")
    ELSE()
        PROJECT("utils_lexbor_encoding" VERSION "${LEXBOR_VERSION_STRING}" LANGUAGES "C")
    ENDIF()
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
