cmake_minimum_required(VERSION 2.8)

project(test_lexbor_encoding)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_ENCODING_SOURCES "*.c")

################
## ARGS for tests
#########################
set(big5_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/big5_map_decode.txt")
set(euc_jp_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/euc_jp_map_decode.txt")
set(euc_kr_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/euc_kr_map_decode.txt")
set(iso_2022_jp_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/iso_2022_jp_map_decode.txt")
set(shift_jis_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/shift_jis_map_decode.txt")
set(gb18030_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/gb18030_map_decode.txt")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_encoding_" "${TEST_LEXBOR_ENCODING_SOURCES}" ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
APPEND_TESTS("lexbor_encoding_" "${TEST_LEXBOR_ENCODING_SOURCES}")
