cmake_minimum_required(VERSION 3.0)

project(lexbor_unit)

################
## Unit
#########################
set(TEST_UNIT_NAME "unit")
set(TEST_UNIT_SOURCE_DIR "unit")
set(TEST_UNIT_LIB_NAME "${TEST_UNIT_NAME}")
set(TEST_UNIT_LIB_NAME_STATIC "${TEST_UNIT_NAME}_static")

################
## Unit Sources
#########################
file(GLOB_RECURSE TEST_UNIT_SOURCES "${TEST_UNIT_SOURCE_DIR}/*.c")

################
## Create Unit libraries
#########################
add_library(${TEST_UNIT_LIB_NAME_STATIC} STATIC ${TEST_UNIT_SOURCES})
target_link_libraries(${TEST_UNIT_LIB_NAME_STATIC} ${LEXBOR_LIB_NAME})

add_library(${TEST_UNIT_LIB_NAME} SHARED ${TEST_UNIT_SOURCES})
target_link_libraries(${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
