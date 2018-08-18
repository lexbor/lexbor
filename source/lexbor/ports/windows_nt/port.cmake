if(NOT DEFINED LEXBOR_OPTIMIZATION_LEVEL)
	set(LEXBOR_OPTIMIZATION_LEVEL "-O2")
endif()

if (!UNIX AND WIN32)
    if(${CMAKE_CL_64})
        add_definitions(-D_WIN64)
    else()
        add_definitions(-D_WIN32)
    endif()
endif()

if(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
    add_definitions(/wd4100 /wd4255 /wd4820 /wd4668)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    message(STATUS "Set Windows definitions")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LEXBOR_OPTIMIZATION_LEVEL} ${LEXBOR_C_FLAGS}")
