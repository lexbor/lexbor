cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Variables
#########################
set(WASM_DEPS_LIB_NAMES ${LEXBOR_LIB_NAME_STATIC})
set(WASM_GEN_SCRIPT "${CMAKE_SOURCE_DIR}/utils/wasm/gen_constants.py")

find_package(Python3 COMPONENTS Interpreter QUIET)

IF(NOT Python3_FOUND)
    message(SEND_ERROR "WASM: python3 not found. JS constants will not be generated.")
ENDIF()

################
## Subs
#########################
FIND_AND_APPEND_SUB_DIRS("lexbor" OFF)
