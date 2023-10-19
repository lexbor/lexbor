cmake_minimum_required(VERSION 2.8.12...3.27)

IF(CMAKE_VERSION VERSION_LESS "3.0")
    IF(LEXBOR_BUILD_TESTS_CPP)
        PROJECT("lexbor_grammar")
    ELSE()
        PROJECT("lexbor_grammar" "C")
    ENDIF()
ELSE()
    cmake_policy(SET CMP0048 NEW)

    IF(LEXBOR_BUILD_TESTS_CPP)
        PROJECT("lexbor_grammar" VERSION "${LEXBOR_VERSION_STRING}")
    ELSE()
        PROJECT("lexbor_grammar" VERSION "${LEXBOR_VERSION_STRING}" LANGUAGES "C")
    ENDIF()
ENDIF()

# From /utils/lexbor/css/grammar.txt
# Save to /test/files/lexbor/css/declarations

################
## Grammar
#########################
set(TEST_GRAMMAR_NAME "grammar")
set(TEST_GRAMMAR_SOURCE_DIR ".")
set(TEST_GRAMMAR_LIB_NAME "${TEST_GRAMMAR_NAME}")
set(TEST_GRAMMAR_LIB_NAME_STATIC "${TEST_GRAMMAR_NAME}_static")

################
## Grammar Sources
#########################
file(GLOB_RECURSE TEST_GRAMMAR_SOURCES "${TEST_GRAMMAR_SOURCE_DIR}/*.c")
list(REMOVE_ITEM TEST_GRAMMAR_SOURCES "${TEST_GRAMMAR_SOURCE_DIR}/grammar.c")
set(UTILS_LEXBOR_GRAMMAR_SOURCES "${TEST_GRAMMAR_SOURCE_DIR}/grammar.c")

################
## Create Grammar libraries
#########################
IF(LEXBOR_BUILD_SHARED)
    add_library(${TEST_GRAMMAR_LIB_NAME} SHARED ${TEST_GRAMMAR_SOURCES})
    target_link_libraries(${TEST_GRAMMAR_LIB_NAME} ${LEXBOR_LIB_NAME})
    target_compile_definitions(${TEST_GRAMMAR_LIB_NAME} PRIVATE "LEXBOR_BUILDING")
ENDIF()

IF(LEXBOR_BUILD_STATIC)
    add_library(${TEST_GRAMMAR_LIB_NAME_STATIC} STATIC ${TEST_GRAMMAR_SOURCES})
    set_target_properties(${TEST_GRAMMAR_LIB_NAME_STATIC} PROPERTIES OUTPUT_NAME ${TEST_GRAMMAR_LIB_NAME_STATIC})
    set_target_properties(${TEST_GRAMMAR_LIB_NAME_STATIC} PROPERTIES
        COMPILE_PDB_NAME "${TEST_GRAMMAR_LIB_NAME_STATIC}"
        COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    target_link_libraries(${TEST_GRAMMAR_LIB_NAME_STATIC} ${LEXBOR_LIB_NAME_STATIC})
    target_compile_definitions(${TEST_GRAMMAR_LIB_NAME_STATIC} PRIVATE "LEXBOR_STATIC")
ENDIF()

################
## Create Utils
#########################
IF(LEXBOR_BUILD_SHARED)
    EXECUTABLE_LIST("utils_grammar" "${UTILS_LEXBOR_GRAMMAR_SOURCES}" ${TEST_GRAMMAR_LIB_NAME} ${LEXBOR_LIB_NAME})
ELSEIF(LEXBOR_BUILD_STATIC)
    EXECUTABLE_LIST("utils_grammar" "${UTILS_LEXBOR_GRAMMAR_SOURCES}" ${TEST_GRAMMAR_LIB_NAME_STATIC} ${LEXBOR_LIB_NAME})
ENDIF()
