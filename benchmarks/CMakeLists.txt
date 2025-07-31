cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Search and Includes
#########################
include_directories(".")

################
## Variables
#########################
IF(LEXBOR_BUILD_SHARED)
    set(BENCHMARKS_DEPS_LIB_NAMES ${LEXBOR_LIB_NAME})
ELSEIF(LEXBOR_BUILD_STATIC)
    set(BENCHMARKS_DEPS_LIB_NAMES ${LEXBOR_LIB_NAME_STATIC})
ELSE()
    message(FATAL_ERROR "Requires static or dynamic Lexbor library")
ENDIF()

################
## Subs
#########################
FIND_AND_APPEND_SUB_DIRS("lexbor" ON)
