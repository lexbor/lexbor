cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_STYLES_SOURCES "*.c")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_STYLES_SOURCES}" ${EXAMPLES_DEPS_LIB_NAMES})

IF(LEXBOR_BUILD_TESTS)
    APPEND_TESTS("examples_lexbor_styles_" "${EXAMPLES_LEXBOR_STYLES_SOURCES}")
ENDIF()