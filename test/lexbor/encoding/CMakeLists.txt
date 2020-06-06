cmake_minimum_required(VERSION 2.8)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB TEST_LEXBOR_ENCODING_FUZZ_SOURCES "fuzz_*.c")
file(GLOB_RECURSE TEST_LEXBOR_ENCODING_BUFFER_SOURCES "buffer/*.c")
file(GLOB_RECURSE TEST_LEXBOR_ENCODING_SINGLE_SOURCES "single/*.c")

################
## ARGS for tests
#########################
set(buffer_big5_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/big5_map_decode.txt")
set(buffer_euc_jp_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/euc_jp_map_decode.txt")
set(buffer_euc_kr_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/euc_kr_map_decode.txt")
set(buffer_iso_2022_jp_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/iso_2022_jp_map_decode.txt")
set(buffer_shift_jis_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/shift_jis_map_decode.txt")
set(buffer_gb18030_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/gb18030_map_decode.txt")

set(single_big5_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/big5_map_decode.txt")
set(single_euc_jp_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/euc_jp_map_decode.txt")
set(single_euc_kr_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/euc_kr_map_decode.txt")
set(single_iso_2022_jp_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/iso_2022_jp_map_decode.txt")
set(single_shift_jis_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/shift_jis_map_decode.txt")
set(single_gb18030_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/encoding/gb18030_map_decode.txt")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_encoding_" "${TEST_LEXBOR_ENCODING_FUZZ_SOURCES}" ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
APPEND_TESTS("lexbor_encoding_" "${TEST_LEXBOR_ENCODING_FUZZ_SOURCES}")

EXECUTABLE_LIST("lexbor_encoding_" "${TEST_LEXBOR_ENCODING_BUFFER_SOURCES}" ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
APPEND_TESTS("lexbor_encoding_" "${TEST_LEXBOR_ENCODING_BUFFER_SOURCES}")

EXECUTABLE_LIST("lexbor_encoding_" "${TEST_LEXBOR_ENCODING_SINGLE_SOURCES}" ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
APPEND_TESTS("lexbor_encoding_" "${TEST_LEXBOR_ENCODING_SINGLE_SOURCES}")
