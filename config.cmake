################
## Dependencies
#########################
IF(NOT LEXBOR_WITHOUT_THREADS)
    set(CMAKE_THREAD_PREFER_PTHREAD 1)
    find_package(Threads REQUIRED)
    IF(NOT CMAKE_USE_PTHREADS_INIT)
        message(FATAL_ERROR "Could NOT find pthreads (missing: CMAKE_USE_PTHREADS_INIT)")
    ENDIF()
ENDIF()

################
## Detect OS type
#########################
set(LEXBOR_PLATFORM "UNDEF")

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    IF(EXISTS "/etc/centos-release")
        set(LEXBOR_PLATFORM "CentOS")
    ENDIF(EXISTS "/etc/centos-release")

    IF(EXISTS "/etc/debian_version")
        set(LEXBOR_PLATFORM "Debian")
    ENDIF(EXISTS "/etc/debian_version")

    IF(EXISTS "/etc/redhat-release")
        set(LEXBOR_PLATFORM "Redhat")
    ENDIF(EXISTS "/etc/redhat-release")

    IF(EXISTS "/etc/fedora-release")
        set(LEXBOR_PLATFORM "Redhat")
    ENDIF(EXISTS "/etc/fedora-release")

    IF(EXISTS "/etc/SuSE-release")
        set(LEXBOR_PLATFORM "SuSe")
    ENDIF(EXISTS "/etc/SuSE-release")

    IF(EXISTS "/etc/gentoo-release")
        set(LEXBOR_PLATFORM "Gentoo")
    ENDIF(EXISTS "/etc/gentoo-release")
ENDIF()

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
MACRO(GET_MODULES_SUB_LIST modules curdir)
    FILE(GLOB children ${curdir}/ ${curdir}/*)

    FOREACH(child ${children})
        string(REGEX MATCH "\\.[^/]+$" MATCHSTR ${child})

        IF(IS_DIRECTORY ${child} AND MATCHSTR STREQUAL "")
            string(REGEX MATCH "[^/]+$" MATCHSTR ${child})

            IF(NOT MATCHSTR STREQUAL "ports")
                LIST(APPEND ${modules} ${MATCHSTR})
            ELSE()
                IF(EXISTS "${child}/${LEXBOR_OS_PORT_NAME}/config.cmake")
                    include("${child}/${LEXBOR_OS_PORT_NAME}/config.cmake")
                ENDIF()
            ENDIF()
        ENDIF()
    ENDFOREACH()
ENDMACRO()

MACRO(GET_MODULES_LIST modules curdir)
    FILE(GLOB children ${curdir}/ ${curdir}/*)

    FOREACH(child ${children})
        string(REGEX MATCH "\\.[^/]+$" MATCHSTR ${child})
        IF(IS_DIRECTORY ${child} AND MATCHSTR STREQUAL "")
            GET_MODULES_SUB_LIST(${modules} ${dirs} ${child})
        ENDIF()
    ENDFOREACH()
ENDMACRO()

MACRO(INCLUDE_MODULE_CONFIG pname module module_dir)
    set(conf_path "${module_dir}/config.cmake")

    set(DEPENDENCIES "")

    IF(EXISTS "${conf_path}")
        set(CURRENT_LIB_NAME "${PROJECT_NAME}-${module}")
        set(CURRENT_LIB_NAME_STATIC "${PROJECT_NAME}-${module}-static")

        include("${conf_path}")
    ENDIF()

    IF(NOT DEPENDENCIES STREQUAL "")
        STRING(REGEX REPLACE "[ \t\n]+" " " DEPENDENCIES ${DEPENDENCIES})
    ENDIF()

    string(TOUPPER ${module} module_upper)
    string(TOUPPER ${pname} pname_upper)

    set(tmp "${pname_upper}_${module_upper}_DEPENDENCIES")
    set(${tmp} "${DEPENDENCIES}")
ENDMACRO()

MACRO(GET_MODULE_DEPENDENCIES pname module result)
    string(TOUPPER ${module} module_upper)
    string(TOUPPER ${pname} pname_upper)

    set(${result} "${${pname_upper}_${module_upper}_DEPENDENCIES}")
ENDMACRO()

MACRO(GET_MODULE_RESURSES headers sources source_dir pname module)
    file(GLOB_RECURSE headers "${source_dir}/${pname}/${module}/*.h")
    file(GLOB_RECURSE sources "${source_dir}/${pname}/${module}/*.c")

    set(port_dir_name "${source_dir}/${pname}/ports/${LEXBOR_OS_PORT_NAME}/${pname}/${module}")

    IF(IS_DIRECTORY ${port_dir_name})
        file(GLOB_RECURSE port_headers "${port_dir_name}/*.h")
        file(GLOB_RECURSE port_sources "${port_dir_name}/*.c")

        LIST(APPEND ${headers} ${port_headers})
        LIST(APPEND ${sources} ${port_sources})
    ENDIF()
ENDMACRO()

MACRO(GET_MODULE_VERSION major minor patch vstr source_dir pname module)
    string(TOUPPER ${module} module_upper)

    set(version_file "${source_dir}/${pname}/${module}/base.h")
    set(version_prefix "LXB_${module_upper}_VERSION")

    IF(module STREQUAL "core")
        set(version_prefix "LEXBOR_VERSION")
    ENDIF()

    file(STRINGS ${version_file} VERSION_PARTS
         REGEX "^#define[ \t]+${version_prefix}_(MAJOR|MINOR|PATCH)[ \t]+[0-9]+$")

    list(GET VERSION_PARTS 0 VERSION_MAJOR_PART)
    list(GET VERSION_PARTS 1 VERSION_MINOR_PART)
    list(GET VERSION_PARTS 2 VERSION_PATCH_PART)

    string(REGEX REPLACE "#define[ \t]+${version_prefix}_MAJOR[ \t]+([0-9]+).*" "\\1" A ${VERSION_MAJOR_PART})
    string(REGEX REPLACE "#define[ \t]+${version_prefix}_MINOR[ \t]+([0-9]+).*" "\\1" B ${VERSION_MINOR_PART})
    string(REGEX REPLACE "#define[ \t]+${version_prefix}_PATCH[ \t]+([0-9]+).*" "\\1" C ${VERSION_PATCH_PART})

    set(${major} ${A})
    set(${minor} ${B})
    set(${patch} ${C})
    set(${vstr} "${A}.${B}.${C}")
ENDMACRO()

MACRO(GET_LEXBOR_VERSION major minor patch vstr)
    file(STRINGS ${LEXBOR_VERSION_FILEPATH} version_part
         REGEX "^LEXBOR_VERSION[ \t]*=[ \t]*[0-9.]+$")

    string(REGEX REPLACE "LEXBOR_VERSION=([0-9]+).*" "\\1" A ${version_part})
    string(REGEX REPLACE "LEXBOR_VERSION[ \t]*=[ \t]*[0-9]+.([0-9]+).*" "\\1" B ${version_part})
    string(REGEX REPLACE "LEXBOR_VERSION[ \t]*=[ \t]*[0-9]+.[0-9]+.([0-9]+).*" "\\1" C ${version_part})

    set(${major} ${A})
    set(${minor} ${B})
    set(${patch} ${C})
    set(${vstr} "${A}.${B}.${C}")
ENDMACRO()

MACRO(SET_MODULE_LIB_DEPENDENCIES libname deps postfix)
    IF(NOT ${deps} EQUAL "")
        string(REGEX REPLACE " +" ";" dep_list ${deps})

        FOREACH(dep ${dep_list})
            set(dep_libname "${PROJECT_NAME}-${dep}${postfix}")

            add_dependencies(${libname} "${dep_libname}")
            target_link_libraries(${libname} "${dep_libname}")
        ENDFOREACH()
    ENDIF()
ENDMACRO()

MACRO(ADD_MODULE_LIBRARY type libname version_string major sources)
    IF(NOT ${type} STREQUAL "")
        add_library(${libname} ${type} ${sources})
    ENDIF()

    set(postfix "")
    set(dflags "-DLEXBOR_BUILDING")

    IF(NOT ${type} STREQUAL "")
        IF(${type} STREQUAL "STATIC")
            set(dflags "-DLEXBOR_STATIC")
            set(postfix "-static")
        ENDIF()
    ENDIF()

    target_link_libraries(${libname} ${CMAKE_THREAD_LIBS_INIT})
    set_target_properties(${libname} PROPERTIES OUTPUT_NAME ${libname})
    set_target_properties(${libname} PROPERTIES VERSION ${version_string} SOVERSION ${major})
    set_property(TARGET ${libname} APPEND PROPERTY COMPILE_FLAGS "${dflags}")

    install(TARGETS ${libname}
            RUNTIME DESTINATION "${LEXBOR_INSTALL_DLL_EXE_DIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
ENDMACRO()

MACRO(INSTALL_MODULE_HEADERS header_path pname module)
    set(dir_search "${header_path}/${pname}/${module}")

    IF(NOT IS_DIRECTORY "${dir_search}")
        message(FATAL_ERROR "Install headers: \"${dir_search}\" in not a directory.")
    ENDIF()

    install(DIRECTORY "${dir_search}" DESTINATION "include/${pname}"
            FILES_MATCHING PATTERN "*.h")

    file(GLOB header_dirs "${dir_search}/*")

    FOREACH(item ${header_dirs})
        IF(IS_DIRECTORY "${item}")
            install(DIRECTORY "${item}" DESTINATION "include/${pname}/${module}"
                    FILES_MATCHING PATTERN "*.h")
        ENDIF()
    ENDFOREACH()
ENDMACRO()

MACRO(MODULE_PRINT module version dep)
    message(STATUS "Append module: ${module} (${version})")

    IF(LEXBOR_PRINT_MODULE_DEPENDENCIES)
        IF(NOT ${dep} EQUAL "")
            message("   dependencies: ${dep}")
        ENDIF()
    ENDIF()
ENDMACRO()

MACRO(EXECUTABLE_LIST name_prefix sources)
    FOREACH(src ${sources})
        get_filename_component(barename ${src} NAME_WE)
        get_filename_component(build_dir ${src} DIRECTORY)

        STRING(REGEX REPLACE "^${LEXBOR_DIR_ROOT}" "" build_dir ${build_dir})
        STRING(REGEX REPLACE "^/+" "" build_dir ${build_dir})
        STRING(REGEX REPLACE "/+" "_" build_exe ${build_dir})

        set(exe_name "${name_prefix}${build_exe}_${barename}")

        add_executable("${exe_name}" ${src})
        set_target_properties("${exe_name}" PROPERTIES
                              RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${build_dir}"
                              OUTPUT_NAME "${barename}")

        add_dependencies("${exe_name}" ${ARGN})
        target_link_libraries("${exe_name}" ${ARGN})
    ENDFOREACH()
ENDMACRO()

MACRO(APPEND_TESTS name_prefix sources)
    FOREACH(src ${sources})
        get_filename_component(barename ${src} NAME_WE)
        get_filename_component(build_dir ${src} DIRECTORY)

        STRING(REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}" "/" relative_path ${build_dir})
        set(relative_path "${relative_path}/${barename}")

        STRING(REGEX REPLACE "/+" "_" arg_name ${relative_path})
        STRING(REGEX REPLACE "^_+" "" arg_name ${arg_name})

        STRING(REGEX REPLACE "^${LEXBOR_DIR_ROOT}" "" build_dir ${build_dir})
        STRING(REGEX REPLACE "^/+" "" build_dir ${build_dir})

        add_test("${name_prefix}${barename}" "${CMAKE_BINARY_DIR}/${build_dir}/${barename}"
                 "${${arg_name}_arg}")

        FOREACH(i RANGE 100)
            IF("${${name_prefix}${arg_name}_arg_${i}}" STREQUAL "")
                break()
            ENDIF()

            add_test("${name_prefix}${barename}_${i}" "${CMAKE_BINARY_DIR}/${build_dir}/${barename}"
                     "${${arg_name}_arg_${i}}")
        ENDFOREACH()
    ENDFOREACH()
ENDMACRO()

MACRO(FIND_AND_APPEND_SUB_DIRS npath skip_error)
    FILE(GLOB children ${npath}/ ${npath}/*)

    FOREACH(child ${children})
        IF(${skip_error} AND NOT EXISTS "${child}/CMakeLists.txt")
            CONTINUE()
        ENDIF()

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
IF(LEXBOR_WITHOUT_THREADS)
    message(STATUS "Build without Threads")
    add_definitions(-DLEXBOR_WITHOUT_THREADS)
ELSE()
    message(STATUS "Build with Threads")
ENDIF()
