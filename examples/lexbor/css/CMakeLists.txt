cmake_minimum_required(VERSION 2.8)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_CSS_SOURCES "*.c")

################
## ARGS for tests
#########################
set(syntax_tokenizer_parse_file_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/css/lexbor.css")

################
## Create executable
#########################
IF(LEXBOR_BUILD_SHARED)
    EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_CSS_SOURCES}" ${LEXBOR_LIB_NAME})

    IF(LEXBOR_BUILD_TESTS)
        APPEND_TESTS("examples_lexbor_css_" "${EXAMPLES_LEXBOR_CSS_SOURCES}")
    ENDIF()
ENDIF()
