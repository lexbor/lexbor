include(CheckFunctionExists)

################
## Check Math functions
#########################
MACRO(FEATURE_TRY_FUNCTION_EXISTS target fname lib_name)
    CHECK_FUNCTION_EXISTS(${fname} test_result)

    IF(NOT test_result)
        unset(test_result CACHE)

        list(APPEND CMAKE_REQUIRED_LIBRARIES ${lib_name})

        CHECK_FUNCTION_EXISTS(${fname} test_result)

        STRING(REGEX REPLACE "${lib_name}" "" CMAKE_REQUIRED_LIBRARIES
               ${CMAKE_REQUIRED_LIBRARIES})

        IF(test_result)
            target_link_libraries(${target} ${lib_name})
        ELSE()
            message(FATAL_ERROR "checking for ${fname}() ... not found")
        ENDIF()
    ENDIF()

    unset(test_result CACHE)
ENDMACRO()

MACRO(FEATURE_CHECK_ASAN out_result)
    set(lexbor_asan_flags "-O0 -g -fsanitize=address -fno-omit-frame-pointer")

    IF(LEXBOR_BUILD_WITH_ASAN)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${lexbor_asan_flags}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${lexbor_asan_flags}")
    ENDIF()

    set(feature_filename "${CMAKE_BINARY_DIR}/feature_check.c")

    set(FEATUTE_CHECK_STRING "
#include <sanitizer/asan_interface.h>

int main(void) {
    return
    #ifdef __SANITIZE_ADDRESS__
        #if defined(ASAN_POISON_MEMORY_REGION) && defined(ASAN_UNPOISON_MEMORY_REGION)
            0;
        #endif
    #else
        #if defined(__has_feature)
            #if __has_feature(address_sanitizer)
                #if defined(ASAN_POISON_MEMORY_REGION) && defined(ASAN_UNPOISON_MEMORY_REGION)
                    0;
                #endif
            #endif
        #endif
    #endif
}")

    file(WRITE ${feature_filename} "${FEATUTE_CHECK_STRING}")

    try_compile(${out_result} "${CMAKE_BINARY_DIR}" "${feature_filename}"
        CMAKE_FLAGS "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
    )

    IF(${${out_result}})
        message(STATUS "Feature ASAN: enabled")
    ELSE()
        message(STATUS "Feature ASAN: disable")
    ENDIF()

    file(REMOVE ${feature_filename})

    IF(LEXBOR_BUILD_WITH_ASAN)
        IF(NOT ${${out_result}})
            STRING(REGEX REPLACE " ${lexbor_asan_flags}" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
            STRING(REGEX REPLACE " ${lexbor_asan_flags}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        ELSE()
            message(STATUS "Updated CFLAGS: ${CMAKE_C_FLAGS}")
            message(STATUS "Updated CXXFLAGS: ${CMAKE_CXX_FLAGS}")
        ENDIF()
    ENDIF()

    unset(lexbor_asan_flags)
    unset(FEATUTE_CHECK_STRING)
    unset(feature_filename)
ENDMACRO()

MACRO(FEATURE_CHECK_FUZZER out_result)
    set(lexbor_fuzzer_flags "-O0 -g -fsanitize=fuzzer")

    IF(LEXBOR_BUILD_WITH_FUZZER)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${lexbor_fuzzer_flags}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${lexbor_fuzzer_flags}")
    ENDIF()

    set(feature_filename "${CMAKE_BINARY_DIR}/feature_check.c")

    set(FEATUTE_CHECK_STRING "
#include <sanitizer/asan_interface.h>

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    return 0;
}")

    file(WRITE ${feature_filename} "${FEATUTE_CHECK_STRING}")

    try_compile(${out_result} "${CMAKE_BINARY_DIR}" "${feature_filename}"
        CMAKE_FLAGS "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
    )

    IF(${${out_result}})
        message(STATUS "Feature Fuzzer: enabled")
    ELSE()
        message(STATUS "Feature Fuzzer: disable")
    ENDIF()

    file(REMOVE ${feature_filename})

    IF(LEXBOR_BUILD_WITH_FUZZER)
        IF(NOT ${${out_result}})
            STRING(REGEX REPLACE " ${lexbor_fuzzer_flags}" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
            STRING(REGEX REPLACE " ${lexbor_fuzzer_flags}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        ELSE()
            message(STATUS "Updated CFLAGS: ${CMAKE_C_FLAGS}")
            message(STATUS "Updated CXXFLAGS: ${CMAKE_CXX_FLAGS}")
        ENDIF()
    ENDIF()

    unset(lexbor_fuzzer_flags)
    unset(FEATUTE_CHECK_STRING)
    unset(feature_filename)
ENDMACRO()
