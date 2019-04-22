cmake_minimum_required(VERSION 2.8)

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
set_property(TARGET ${TEST_UNIT_LIB_NAME_STATIC} APPEND PROPERTY COMPILE_FLAGS "-DLEXBOR_STATIC")

add_library(${TEST_UNIT_LIB_NAME} SHARED ${TEST_UNIT_SOURCES})
target_link_libraries(${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
set_property(TARGET ${TEST_UNIT_LIB_NAME} APPEND PROPERTY COMPILE_FLAGS "-DLEXBOR_BUILDING")

if (WIN32)
    add_custom_command(TARGET ${TEST_UNIT_LIB_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/lib${TEST_UNIT_LIB_NAME}.dll" $<TARGET_FILE_DIR:${LEXBOR_LIB_NAME}>/test/lexbor/core
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/lib${TEST_UNIT_LIB_NAME}.dll" $<TARGET_FILE_DIR:${LEXBOR_LIB_NAME}>/test/lexbor/html
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/lib${TEST_UNIT_LIB_NAME}.dll" $<TARGET_FILE_DIR:${LEXBOR_LIB_NAME}>/test/lexbor/ns
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/lib${TEST_UNIT_LIB_NAME}.dll" $<TARGET_FILE_DIR:${LEXBOR_LIB_NAME}>/test/lexbor/tag)
endif (WIN32)
