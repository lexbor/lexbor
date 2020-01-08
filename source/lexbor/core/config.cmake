set(DEPENDENCIES "")
set(DESCRIPTION "The main module of the Lexbor project.
The module includes various algorithms and methods for working with memory:
AVL Tree, Array, String, Memory Pool and so on.")

IF(LEXBOR_BUILD_SEPARATELY)
    FEATURE_TRY_FUNCTION_EXISTS("${CURRENT_LIB_NAME}" "ceil" "m")
ENDIF()

IF(TARGET ${LEXBOR_LIB_NAME})
    FEATURE_TRY_FUNCTION_EXISTS("${LEXBOR_LIB_NAME}" "ceil" "m")
ENDIF()