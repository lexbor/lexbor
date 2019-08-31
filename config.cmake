################
## Dependencies
#########################
if(NOT LEXBOR_WITHOUT_THREADS)
    set(CMAKE_THREAD_PREFER_PTHREAD 1)
    find_package(Threads REQUIRED)
    if(NOT CMAKE_USE_PTHREADS_INIT)
        message(FATAL_ERROR "Could NOT find pthreads (missing: CMAKE_USE_PTHREADS_INIT)")
    endif()
endif()

################
## Detect OS type
#########################
set(LEXBOR_PLATFORM "UNDEF")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(EXISTS "/etc/centos-release")
        set(LEXBOR_PLATFORM "CentOS")
    endif(EXISTS "/etc/centos-release")

    if(EXISTS "/etc/debian_version")
        set(LEXBOR_PLATFORM "Debian")
    endif(EXISTS "/etc/debian_version")

    if(EXISTS "/etc/redhat-release")
        set(LEXBOR_PLATFORM "Redhat")
    endif(EXISTS "/etc/redhat-release")

    if(EXISTS "/etc/fedora-release")
        set(LEXBOR_PLATFORM "Redhat")
    endif(EXISTS "/etc/fedora-release")

    if(EXISTS "/etc/SuSE-release")
        set(LEXBOR_PLATFORM "SuSe")
    endif(EXISTS "/etc/SuSE-release")

    if(EXISTS "/etc/gentoo-release")
        set(LEXBOR_PLATFORM "Gentoo")
    endif(EXISTS "/etc/gentoo-release")
endif()

################
## OS ports
#########################
if (WIN32)
    set(LEXBOR_OS_PORT_NAME "windows_nt")
else ()
    set(LEXBOR_OS_PORT_NAME "posix")
endif (WIN32)

################
## Macro
#########################
MACRO(GET_MODULES_SUB_LIST modules dirs curdir)
    FILE(GLOB children ${curdir}/ ${curdir}/*)

    FOREACH(child ${children})
        string(REGEX MATCH "\\.[^/]+$" MATCHSTR ${child})

        IF(IS_DIRECTORY ${child} AND MATCHSTR STREQUAL "")
            string(REGEX MATCH "[^/]+$" MATCHSTR ${child})
            IF(NOT MATCHSTR STREQUAL "ports")
                LIST(APPEND ${modules} ${MATCHSTR})
                LIST(APPEND ${dirs} ${child})
            ELSE()
                IF(EXISTS "${child}/${LEXBOR_OS_PORT_NAME}/port.cmake")
                    message(STATUS "Include port: ${child}/${LEXBOR_OS_PORT_NAME}")
                    include("${child}/${LEXBOR_OS_PORT_NAME}/port.cmake")
                ENDIF()

                LIST(APPEND ${dirs} "${child}/${LEXBOR_OS_PORT_NAME}")
            ENDIF()
        ENDIF()
    ENDFOREACH()
ENDMACRO()

MACRO(GET_MODULES_LIST modules dirs curdir)
    FILE(GLOB children ${curdir}/ ${curdir}/*)

    FOREACH(child ${children})
        string(REGEX MATCH "\\.[^/]+$" MATCHSTR ${child})
        IF(IS_DIRECTORY ${child} AND MATCHSTR STREQUAL "")
            GET_MODULES_SUB_LIST(${modules} ${dirs} ${child})
        ENDIF()
    ENDFOREACH()
ENDMACRO()

MACRO(CREATE_MODULES_SOURCES result dirs)
    FOREACH(dir ${dirs})
        LIST(APPEND ${result} ${dir}/*.c)
    ENDFOREACH()
ENDMACRO()

MACRO(MODULES_PRINT modules)
    FOREACH(module ${modules})
        message(STATUS "Append module: ${module}")
    ENDFOREACH()
ENDMACRO()

MACRO(EXECUTABLE_LIST name_prefix sources)
    FOREACH(src ${sources})
        get_filename_component(barename ${src} NAME_WE)
        get_filename_component(build_dir ${src} DIRECTORY)

        STRING(REGEX REPLACE "^${LEXBOR_DIR_ROOT}" "" build_dir ${build_dir})
        STRING(REGEX REPLACE "^/+" "" build_dir ${build_dir})

        add_executable("${name_prefix}${barename}" ${src})
        set_target_properties("${name_prefix}${barename}" PROPERTIES
                              RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${build_dir}"
                              OUTPUT_NAME "${barename}")
        target_link_libraries("${name_prefix}${barename}" ${ARGN})
    ENDFOREACH()
ENDMACRO()

MACRO(APPEND_TESTS name_prefix sources)
    FOREACH(src ${sources})
        get_filename_component(barename ${src} NAME_WE)
        add_test("${name_prefix}${barename}" "${barename}" "${${name_prefix}${barename}_arg}")

        FOREACH(i RANGE 100)
            IF("${${name_prefix}${barename}_arg_${i}}" STREQUAL "")
                break()
            ENDIF()

            get_filename_component(barename ${src} NAME_WE)
            add_test("${name_prefix}${barename}_${i}" "${name_prefix}${barename}" "${${name_prefix}${barename}_arg_${i}}")
        ENDFOREACH()
    ENDFOREACH()
ENDMACRO()

MACRO(FIND_AND_APPEND_SUB_DIRS npath)
    FILE(GLOB children ${npath}/ ${npath}/*)

    FOREACH(child ${children})
        string(REGEX MATCH "\\.[^/]+$" MATCHSTR ${child})
        IF(IS_DIRECTORY ${child} AND MATCHSTR STREQUAL "")
            string(REGEX MATCH "[^/]+$" MATCHSTR ${child})
            IF(NOT MATCHSTR STREQUAL "ports")
                add_subdirectory(${child})
            ELSE()
                IF(EXISTS "${child}/${LEXBOR_OS_PORT_NAME}/port.cmake")
                    message(STATUS "Include port: ${child}/${LEXBOR_OS_PORT_NAME}")
                    include("${child}/${LEXBOR_OS_PORT_NAME}/port.cmake")
                ENDIF()

                add_subdirectory("${child}/${LEXBOR_OS_PORT_NAME}")
            ENDIF()
        ENDIF()
    ENDFOREACH()
ENDMACRO()

################
## ARGS
#########################
if(LEXBOR_WITHOUT_THREADS)
  message(STATUS "Build without Threads")
  add_definitions(-DLEXBOR_WITHOUT_THREADS)
else()
  message(STATUS "Build with Threads")
endif()
