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
    set(lexbor_asan_flags "-O0 -g -fsanitize=undefined,address -fno-omit-frame-pointer")

    IF(LEXBOR_BUILD_WITH_ASAN)
        IF(LEXBOR_BUILD_WITH_MSAN)
            message(FATAL_ERROR "Attempt to build a project with two types of sanitize: LEXBOR_BUILD_WITH_MSAN and LEXBOR_BUILD_WITH_ASAN")
        ENDIF()

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

MACRO(FEATURE_CHECK_MSAN out_result)
    set(lexbor_msan_flags "-O0 -g -fsanitize=memory -fno-omit-frame-pointer")

    IF(LEXBOR_BUILD_WITH_MSAN)
        IF(LEXBOR_BUILD_WITH_ASAN)
            message(FATAL_ERROR "Attempt to build a project with two types of sanitize: LEXBOR_BUILD_WITH_MSAN and LEXBOR_BUILD_WITH_ASAN")
        ENDIF()

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${lexbor_msan_flags}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${lexbor_msan_flags}")
    ENDIF()

    set(feature_filename "${CMAKE_BINARY_DIR}/feature_check.c")

    set(FEATUTE_CHECK_STRING "
#include <stdio.h>

int main(void) {
    return
        #if defined(__has_feature)
            #if __has_feature(memory_sanitizer)
                0;
            #endif
        #endif
}")

    file(WRITE ${feature_filename} "${FEATUTE_CHECK_STRING}")

    try_compile(${out_result} "${CMAKE_BINARY_DIR}" "${feature_filename}"
        CMAKE_FLAGS "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
    )

    IF(${${out_result}})
        message(STATUS "Feature MSAN: enabled")
    ELSE()
        message(STATUS "Feature MSAN: disable")
    ENDIF()

    file(REMOVE ${feature_filename})

    IF(LEXBOR_BUILD_WITH_MSAN)
        IF(NOT ${${out_result}})
            STRING(REGEX REPLACE " ${lexbor_msan_flags}" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
            STRING(REGEX REPLACE " ${lexbor_msan_flags}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        ELSE()
            message(STATUS "Updated CFLAGS: ${CMAKE_C_FLAGS}")
            message(STATUS "Updated CXXFLAGS: ${CMAKE_CXX_FLAGS}")
        ENDIF()
    ENDIF()

    unset(lexbor_msan_flags)
    unset(FEATUTE_CHECK_STRING)
    unset(feature_filename)
ENDMACRO()

MACRO(FEATURE_CHECK_FUZZER out_result)
    IF(NOT LEXBOR_BUILD_WITH_FUZZER)
        message(STATUS "Feature Fuzzer: disable")
    ELSE()
        set(lexbor_old_c_flags "${CMAKE_C_FLAGS}")
        set(lexbor_old_cxx_flags "${CMAKE_CXX_FLAGS}")
        set(lexbor_fuzzer_flags "-O0 -g -fsanitize=fuzzer")
        set(feature_filename "${CMAKE_BINARY_DIR}/feature_check.c")

        STRING(REGEX REPLACE " ?-O[0-9]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        STRING(REGEX REPLACE " ?-O[0-9]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${lexbor_fuzzer_flags}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${lexbor_fuzzer_flags}")

        set(FEATUTE_CHECK_STRING "
#include <sanitizer/asan_interface.h>

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    return 0;
}
")

        file(WRITE ${feature_filename} "${FEATUTE_CHECK_STRING}")

        try_compile(${out_result} "${CMAKE_BINARY_DIR}" "${feature_filename}"
            CMAKE_FLAGS "${lexbor_fuzzer_flags}"
            OUTPUT_VARIABLE OUTPUT
        )

        IF(${out_result})
            message(STATUS "Feature Fuzzer: enabled")
            message(STATUS "Updated CFLAGS: ${CMAKE_C_FLAGS}")
            message(STATUS "Updated CXXFLAGS: ${CMAKE_CXX_FLAGS}")

            set(${out_result} TRUE)
        ELSE()
            set(CMAKE_C_FLAGS ${lexbor_old_c_flags})
            set(CMAKE_CXX_FLAGS ${lexbor_old_cxx_flags})

            message(STATUS "Feature Fuzzer: аn error was received at compilation.")
            message(STATUS "Feature Fuzzer OUTPUT:")
            message(FATAL_ERROR ${OUTPUT})

            set(${out_result} FALSE)
        ENDIF()

        file(REMOVE ${feature_filename})

        unset(lexbor_old_c_flags)
        unset(lexbor_old_cxx_flags)
        unset(lexbor_fuzzer_flags)
        unset(FEATUTE_CHECK_STRING)
        unset(feature_filename)
    ENDIF()
ENDMACRO()
