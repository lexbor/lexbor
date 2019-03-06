include(CheckFunctionExists)

################
## Check Math functions
#########################
CHECK_FUNCTION_EXISTS(ceil LEXBOR_FEATURE_CEIL)
if(NOT LEXBOR_FEATURE_CEIL)
    unset(LEXBOR_FEATURE_CEIL CACHE)

    list(APPEND CMAKE_REQUIRED_LIBRARIES m)

    CHECK_FUNCTION_EXISTS(ceil LEXBOR_FEATURE_CEIL)
    if(NOT LEXBOR_FEATURE_CEIL)
        target_link_libraries(${LEXBOR_LIB_NAME} m)
        target_link_libraries(${LEXBOR_LIB_NAME_STATIC} m)
    else()
        message(FATAL_ERROR "checking for ceil() ... not found")
    endif()
endif()
