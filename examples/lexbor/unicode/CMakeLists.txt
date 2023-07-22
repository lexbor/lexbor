cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_UNICODE_SOURCES "*.c")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_UNICODE_SOURCES}" ${EXAMPLES_DEPS_LIB_NAMES})

IF(LEXBOR_BUILD_TESTS)
    APPEND_TESTS("examples_lexbor_unicode_" "${EXAMPLES_LEXBOR_UNICODE_SOURCES}")
ENDIF()