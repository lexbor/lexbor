cmake_minimum_required(VERSION 2.8.12)

################
## Search and Includes
#########################
include_directories(".")

################
## Includes
#########################
include(unit/CMakeLists.txt)

################
## Variables
#########################
IF(LEXBOR_BUILD_SHARED)
    set(TEST_LEXBOR_LIB_NAME ${LEXBOR_LIB_NAME})
    set(TEST_DEPS_LIB_NAMES ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
ELSEIF(LEXBOR_BUILD_STATIC)
    set(TEST_LEXBOR_LIB_NAME ${LEXBOR_LIB_NAME_STATIC})
    set(TEST_DEPS_LIB_NAMES ${TEST_UNIT_LIB_NAME_STATIC} ${LEXBOR_LIB_NAME_STATIC})
ELSE()
    message(FATAL_ERROR "Requires static or dynamic Lexbor library")
ENDIF()

################
## Subs
#########################
FIND_AND_APPEND_SUB_DIRS("lexbor" OFF)
FIND_AND_APPEND_SUB_DIRS("fuzzers/lexbor" OFF)
