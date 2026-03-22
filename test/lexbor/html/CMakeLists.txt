cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_HTML_SOURCES "*.c")

if(LEXBOR_BUILD_TESTS_CPP)
    file(GLOB_RECURSE TEST_LEXBOR_HTML_SOURCES_CPP "*.cpp")
endif()

################
## ARGS for tests
#########################
set(tokenizer_tokens_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/html/tokenizer")
set(tree_builder_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/html/html5_test")
set(tokenizer_html5lib_tests_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/html/html5lib_tokenizer")
set(encoding_html5lib_tests_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/html/html5lib_encoding")
set(serialize_ext_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/html/serialize_ext")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_html_" "${TEST_LEXBOR_HTML_SOURCES}" ${TEST_DEPS_LIB_NAMES})
APPEND_TESTS("lexbor_html_" "${TEST_LEXBOR_HTML_SOURCES}")

if(LEXBOR_BUILD_TESTS_CPP)
    EXECUTABLE_LIST("lexbor_html_" "${TEST_LEXBOR_HTML_SOURCES_CPP}" ${TEST_DEPS_LIB_NAMES})
    APPEND_TESTS("lexbor_html_" "${TEST_LEXBOR_HTML_SOURCES_CPP}")
endif()
