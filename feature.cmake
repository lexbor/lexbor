include(CheckFunctionExists)

################
## Check Math functions
#########################
MACRO(FEATURE_CHECK_FUNCTION_EXISTS target fname lib_name)
    CHECK_FUNCTION_EXISTS(${fname} test_result)

    IF(NOT test_result)
        unset(test_result CACHE)

        list(APPEND CMAKE_REQUIRED_LIBRARIES ${lib_name})

        CHECK_FUNCTION_EXISTS(${fname} test_result)

        IF(test_result)
            target_link_libraries(${target} ${lib_name})
        ELSE()
            message(FATAL_ERROR "checking for ${fname}() ... not found")
        ENDIF()
    endif()
ENDMACRO()
