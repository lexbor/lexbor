cmake_minimum_required(VERSION 2.8)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_CSS_SOURCES "*.c")

################
## ARGS for tests
#########################
set(syntax_tokenizer_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/syntax/tokenizer")

################
## Create tests
#########################
IF(LEXBOR_BUILD_SHARED)
    EXECUTABLE_LIST("lexbor_css_" "${TEST_LEXBOR_CSS_SOURCES}" ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
    APPEND_TESTS("lexbor_css_" "${TEST_LEXBOR_CSS_SOURCES}")
ENDIF()
