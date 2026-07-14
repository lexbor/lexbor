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

set(TEST_LEXBOR_HTML_CTEST_SOURCES "")
set(TEST_LEXBOR_HTML_WPT_SOURCES "")

foreach(src ${TEST_LEXBOR_HTML_SOURCES})
    get_filename_component(barename ${src} NAME_WE)

    if("${barename}" STREQUAL "wpt_tree_construction")
        list(APPEND TEST_LEXBOR_HTML_WPT_SOURCES ${src})
    else()
        list(APPEND TEST_LEXBOR_HTML_CTEST_SOURCES ${src})
    endif()
endforeach()

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
EXECUTABLE_LIST("lexbor_html_" "${TEST_LEXBOR_HTML_CTEST_SOURCES}" ${TEST_DEPS_LIB_NAMES})
APPEND_TESTS("lexbor_html_" "${TEST_LEXBOR_HTML_CTEST_SOURCES}")

if(LEXBOR_ENABLE_TEST_HTML_WPT)
    find_program(LEXBOR_PYTHON_EXECUTABLE NAMES python3 python)
    if(NOT LEXBOR_PYTHON_EXECUTABLE)
        message(FATAL_ERROR "LEXBOR_ENABLE_TEST_HTML_WPT requires Python 3 interpreter.")
    endif()

    execute_process(
        COMMAND "${LEXBOR_PYTHON_EXECUTABLE}" -c
                "import sys; sys.exit(sys.version_info[0] != 3)"
        RESULT_VARIABLE LEXBOR_PYTHON_VERSION_STATUS)
    if(NOT LEXBOR_PYTHON_VERSION_STATUS EQUAL 0)
        message(FATAL_ERROR "LEXBOR_ENABLE_TEST_HTML_WPT requires Python 3 interpreter.")
    endif()

    EXECUTABLE_LIST("lexbor_html_" "${TEST_LEXBOR_HTML_WPT_SOURCES}" ${TEST_DEPS_LIB_NAMES})

    set(wpt_tree_construction_dest
        "${CMAKE_BINARY_DIR}/test/files/lexbor/html/wpt_tree_construction")
    set(wpt_tree_construction_runner
        "${CMAKE_BINARY_DIR}/test/lexbor/html/wpt_tree_construction${CMAKE_EXECUTABLE_SUFFIX}")

    add_test(NAME "lexbor_html_wpt_tree_construction"
        COMMAND "${LEXBOR_PYTHON_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/utils/lexbor/html/wpt_tree_construction.py"
                "--dest" "${wpt_tree_construction_dest}"
                "--runner" "${wpt_tree_construction_runner}")
endif()

if(LEXBOR_BUILD_TESTS_CPP)
    EXECUTABLE_LIST("lexbor_html_" "${TEST_LEXBOR_HTML_SOURCES_CPP}" ${TEST_DEPS_LIB_NAMES})
    APPEND_TESTS("lexbor_html_" "${TEST_LEXBOR_HTML_SOURCES_CPP}")
endif()
