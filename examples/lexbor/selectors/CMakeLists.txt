cmake_minimum_required(VERSION 2.8)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_SELECTORS "*.c")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_SELECTORS}" ${LEXBOR_LIB_NAME})

IF(LEXBOR_BUILD_TESTS)
    APPEND_TESTS("examples_lexbor_selectors_" "${EXAMPLES_LEXBOR_SELECTORS}")
ENDIF()