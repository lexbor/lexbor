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
MACRO(LIST_JOIN tlist glue out)
    set(value "")
    set(nlist "${${tlist}}")

    FOREACH(entry ${nlist})
        set(value "${entry}${glue}${value}")
    ENDFOREACH()

    STRING(REGEX REPLACE "${glue}\$" " " value ${value})

    set(${out} "${value}")

    unset(value)
    unset(nlist)
ENDMACRO()

MACRO(LIST_TO_COLUMN tlist column join_by out_result)
    set(out "")
    set(line "")

    STRING(STRIP ${join_by} join_by_cls)

    FOREACH(mname ${tlist})
        IF(NOT "${line}" STREQUAL "")
            set(tmp "${line}${join_by}${mname}")
        ELSE()
            set(tmp "${mname}")
        ENDIF()

        STRING(LENGTH "${tmp}" str_len)

        IF("${str_len}" GREATER "${column}" OR "${str_len}" EQUAL "${column}")
            set(out "${out}\n${line}${join_by_cls}")

            set(line "")
            set(tmp "${mname}")
        ENDIF()

        set(line ${tmp})
    ENDFOREACH()

    IF(NOT "${line}" STREQUAL "")
        set(out "${out}\n${line}")
    ENDIF()

    STRING(STRIP ${out} out)
    set(${out_result} ${out})

    unset(line)
    unset(out)
ENDMACRO()

MACRO(SUBDIRLIST_PRIVATE curdir prefix postfix to_list)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)

    FOREACH(child ${children})
        IF(IS_DIRECTORY "${curdir}/${child}")
            LIST(APPEND ${to_list} "${prefix}${child}${postfix}")
            SUBDIRLIST_PRIVATE("${curdir}/${child}" "${prefix}${child}/" "${postfix}" ${to_list})
        ENDIF()
    ENDFOREACH()
ENDMACRO()

MACRO(SUBDIRLIST curdir prefix postfix out_result)
    SUBDIRLIST_PRIVATE("${curdir}" "${prefix}" "${postfix}" dirlist)

    SET(${out_result} ${dirlist})
ENDMACRO()

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

    string(TOUPPER ${module} module_upper)
    string(TOUPPER ${pname} pname_upper)

    set(tmp "${pname_upper}_${module_upper}_INCLUDED")

    IF("${${tmp}}" STREQUAL "")
        set(${tmp} TRUE)

        set(DEPENDENCIES "")

        IF(EXISTS "${conf_path}")
            set(CURRENT_LIB_NAME "${PROJECT_NAME}-${module}")
            set(CURRENT_LIB_NAME_STATIC "${PROJECT_NAME}-${module}-static")

            include("${conf_path}")
        ENDIF()

        IF(NOT DEPENDENCIES STREQUAL "")
            STRING(REGEX REPLACE "[ \t\n]+" " " DEPENDENCIES ${DEPENDENCIES})
        ENDIF()

        set(tmp "${pname_upper}_${module_upper}_DEPENDENCIES")
        set(${tmp} "${DEPENDENCIES}")

        set(tmp "${pname_upper}_${module_upper}_TITLE")
        set(${tmp} "${TITLE}")

        set(tmp "${pname_upper}_${module_upper}_DESCRIPTION")
        set(${tmp} "${DESCRIPTION}")
    ENDIF()
ENDMACRO()

MACRO(GET_MODULE_TITLE pname module result)
    string(TOUPPER ${module} module_upper)
    string(TOUPPER ${pname} pname_upper)

    set(${result} "${${pname_upper}_${module_upper}_TITLE}")

    unset(module_upper)
    unset(pname_upper)
ENDMACRO()

MACRO(GET_MODULE_DESCRIPTION pname module result)
    string(TOUPPER ${module} module_upper)
    string(TOUPPER ${pname} pname_upper)

    set(${result} "${${pname_upper}_${module_upper}_DESCRIPTION}")

    unset(module_upper)
    unset(pname_upper)
ENDMACRO()

MACRO(GET_MODULE_DEPENDENCIES pname module result)
    string(TOUPPER ${module} module_upper)
    string(TOUPPER ${pname} pname_upper)

    set(${result} "${${pname_upper}_${module_upper}_DEPENDENCIES}")

    unset(module_upper)
    unset(pname_upper)
ENDMACRO()

MACRO(GET_MODULE_INCLUDED pname module result)
    string(TOUPPER ${module} module_upper)
    string(TOUPPER ${pname} pname_upper)

    set(${result} "${${pname_upper}_${module_upper}_INCLUDED}")

    unset(module_upper)
    unset(pname_upper)
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

    IF(${module} STREQUAL "core")
        set(version_prefix "LEXBOR_VERSION")
    ENDIF()

    set(version_cache "${version_prefix}_CACHE")

    IF(NOT "${${version_cache}}" STREQUAL "")
        set(${vstr} "${${version_cache}}")
    ELSE()
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

        set(${version_cache} "${${vstr}}")
    ENDIF()
ENDMACRO()

MACRO(GET_LEXBOR_VERSION major minor patch vstr)
    file(STRINGS ${LEXBOR_VERSION_FILEPATH} version_part
         REGEX "^LEXBOR_VERSION[ \t]*=[ \t]*[0-9.]+$")

    string(REGEX REPLACE "LEXBOR_VERSION=([0-9]+).*" "\\1" AV ${version_part})
    string(REGEX REPLACE "LEXBOR_VERSION[ \t]*=[ \t]*[0-9]+.([0-9]+).*" "\\1" BV ${version_part})
    string(REGEX REPLACE "LEXBOR_VERSION[ \t]*=[ \t]*[0-9]+.[0-9]+.([0-9]+).*" "\\1" CV ${version_part})

    set(${major} ${AV})
    set(${minor} ${BV})
    set(${patch} ${CV})
    set(${vstr} "${AV}.${BV}.${CV}")
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

MACRO(ADD_MODULE_LIBRARY type libname version_string major)
    set(dflags "")

    IF(${type} STREQUAL "STATIC")
        set(dflags "-DLEXBOR_STATIC")
    ELSE()
        set(dflags "-DLEXBOR_SHARED")
    ENDIF()

    target_link_libraries(${libname} ${CMAKE_THREAD_LIBS_INIT})
    set_target_properties(${libname} PROPERTIES OUTPUT_NAME ${libname})
    set_target_properties(${libname} PROPERTIES VERSION ${version_string} SOVERSION ${major})
    set_property(TARGET ${libname} APPEND PROPERTY COMPILE_FLAGS "${dflags}")

    install(TARGETS ${libname}
            RUNTIME DESTINATION "${LEXBOR_INSTALL_DLL_EXE_DIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

    unset(dflags)
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
        STRING(REGEX REPLACE "^/+" "" relative_path ${relative_path})

        STRING(REGEX REPLACE "/+" "_" arg_name ${relative_path})
        STRING(REGEX REPLACE "^_+" "" arg_name ${arg_name})

        STRING(REGEX REPLACE "^${LEXBOR_DIR_ROOT}" "" build_dir ${build_dir})
        STRING(REGEX REPLACE "^/+" "" build_dir ${build_dir})

        IF (NOT "${${arg_name}_arg}" STREQUAL "")
            add_test("${name_prefix}${arg_name}" "${CMAKE_BINARY_DIR}/${build_dir}/${barename}"
                     "${${arg_name}_arg}")
        ELSE()
            add_test("${name_prefix}${arg_name}" "${CMAKE_BINARY_DIR}/${build_dir}/${barename}")
        ENDIF()

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

MACRO(MAKE_RPM_SPEC module libname out_result)
    INCLUDE_MODULE_CONFIG(${PROJECT_NAME} ${module} "${LEXBOR_SOURCE_LEXBOR}/${module}")

    GET_MODULE_VERSION(major minor patch version_string
                       "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})
    GET_MODULE_DEPENDENCIES(${PROJECT_NAME} ${module} deps)

    set(requires "")
    set(requires_devel "")

    IF(NOT ${deps} STREQUAL "")
        string(REPLACE " " ";" deps ${deps})
    ENDIF()

    FOREACH(dep ${deps})
        IF(${dep} STREQUAL "")
            CONTINUE()
        ENDIF()

        GET_MODULE_VERSION(dep_major dep_minor dep_patch dep_version_string
                           "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${dep})

        set(dep_libname "${PROJECT_NAME}-${dep}")
        set(requires "${requires}Requires: lib${dep_libname} = %{epoch}:${dep_version_string}-%{release}\n")
        set(requires_devel "${requires_devel}Requires: lib${dep_libname}-devel = %{epoch}:${dep_version_string}-%{release}\n")
    ENDFOREACH()

    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/packaging/rpm/liblexbor-module.spec.in" rpm_module_in)

    GET_MODULE_DESCRIPTION("${PROJECT_NAME}" ${module} desc)

    STRING(REGEX REPLACE "%%NAME%%" "${module}" rpm_module_in ${rpm_module_in})
    STRING(REGEX REPLACE "%%LIBNAME%%" "${libname}" rpm_module_in ${rpm_module_in})
    STRING(REGEX REPLACE "%%VERSION%%" "${version_string}" rpm_module_in ${rpm_module_in})
    STRING(REGEX REPLACE "%%REQUIRES%%" "${requires}" rpm_module_in ${rpm_module_in})
    STRING(REGEX REPLACE "%%REQUIRES_DEVEL%%" "${requires_devel}" rpm_module_in ${rpm_module_in})
    STRING(REGEX REPLACE "%%DESCRIPTION%%" "${desc}" rpm_module_in ${rpm_module_in})

    set(${out_result} ${rpm_module_in})
ENDMACRO()

MACRO(CREATE_RPM_SPEC_FILE)
    set(modules_specs "")
    set(req_modules "")
    set(req_modules_devel "")

    FOREACH(module ${LEXBOR_MODULES})
        set(libname "${PROJECT_NAME}-${module}")

        MAKE_RPM_SPEC(${module} ${libname} module_spec)

        set(modules_specs "${modules_specs}${module_spec}\n")

        GET_MODULE_VERSION(major minor patch version_string
                        "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})

        set(req_modules "${req_modules}Requires: lib${libname} = %{epoch}:${version_string}-%{release}\n")
        set(req_modules_devel "${req_modules_devel}Requires: lib${libname}-devel = %{epoch}:${version_string}-%{release}\n")
    ENDFOREACH()

    STRING(STRIP ${modules_specs} modules_specs)

    set(sorted_modules ${LEXBOR_MODULES})
    LIST(SORT sorted_modules)

    LIST_TO_COLUMN("${sorted_modules}" "80" ", " modules_names)

    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/packaging/rpm/liblexbor.spec.in"
        rpm_spec_in)

    STRING(REGEX REPLACE "%%REQUIRES%%" "${req_modules}" rpm_spec_in ${rpm_spec_in})
    STRING(REGEX REPLACE "%%REQUIRES_DEVEL%%" "${req_modules_devel}" rpm_spec_in ${rpm_spec_in})
    STRING(REGEX REPLACE "%%MODULES_SPECS%%" "${modules_specs}" rpm_spec_in ${rpm_spec_in})
    STRING(REGEX REPLACE "%%MODULES_NAMES%%" "${modules_names}" rpm_spec_in ${rpm_spec_in})

    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/rpm/liblexbor.spec"
        "${rpm_spec_in}")

    unset(modules_specs)
    unset(req_modules)
    unset(req_modules_devel)
ENDMACRO()

MACRO(CREATE_DEB_DIRS with_inc module libname arch debian_in_dir debian_dir)
    file(READ "${debian_in_dir}/dirs" dirs)
    file(READ "${debian_in_dir}/dev.dirs" dev_dirs)
    file(READ "${debian_in_dir}/install" inst)
    file(READ "${debian_in_dir}/dev.install" dev_inst)

    STRING(REGEX REPLACE "%%ARCH%%" "${arch}" dirs "${dirs}")
    STRING(REGEX REPLACE "%%ARCH%%" "${arch}" dev_dirs "${dev_dirs}")
    STRING(REGEX REPLACE "%%ARCH%%" "${arch}" inst "${inst}")
    STRING(REGEX REPLACE "%%ARCH%%" "${arch}" dev_inst "${dev_inst}")
    STRING(REGEX REPLACE "%%LIBNAME%%" "${libname}" inst "${inst}")
    STRING(REGEX REPLACE "%%LIBNAME%%" "${libname}" dev_inst "${dev_inst}")

    IF(${with_inc})
        IF(NOT ${module} STREQUAL "")
            STRING(REGEX REPLACE "%%INCLUDES%%" "usr/include/lexbor/${module}" dev_dirs "${dev_dirs}")
            STRING(REGEX REPLACE "%%INCLUDES%%" "usr/include/lexbor/${module}" dev_inst "${dev_inst}")
        ELSE()
            STRING(REGEX REPLACE "%%INCLUDES%%" "usr/include/lexbor" dev_dirs "${dev_dirs}")
            STRING(REGEX REPLACE "%%INCLUDES%%" "usr/include/lexbor" dev_inst "${dev_inst}")
        ENDIF()
    ELSE()
        STRING(REGEX REPLACE "%%INCLUDES%%" "" dev_dirs "${dev_dirs}")
        STRING(REGEX REPLACE "%%INCLUDES%%" "" dev_inst "${dev_inst}")
    ENDIF()

    file(WRITE "${debian_dir}/lib${libname}.dirs" "${dirs}")
    file(WRITE "${debian_dir}/lib${libname}-dev.dirs" "${dev_dirs}")
    file(WRITE "${debian_dir}/lib${libname}.install" "${inst}")
    file(WRITE "${debian_dir}/lib${libname}-dev.install" "${dev_inst}")

    unset(dirs)
    unset(dev_dirs)
    unset(inst)
    unset(dev_inst)
ENDMACRO()

MACRO(PACKAGE_DEB_CREATE_MAKEFILES)
    set(mkdeps "")
    set(mkmodules "")

    # Create deps for modules
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/Makefile.module.in" mkmodule_in)

    FOREACH(module ${LEXBOR_MODULES})
        set(libname "${PROJECT_NAME}-${module}")
        set(module_out "${mkmodule_in}")
        set(deps_cp "")

        INCLUDE_MODULE_CONFIG(${PROJECT_NAME} ${module} "${LEXBOR_SOURCE_LEXBOR}/${module}")

        GET_MODULE_VERSION(major minor patch version_string
                           "${LEXBOR_SOURCE}" ${PROJECT_NAME} ${module})
        GET_MODULE_DEPENDENCIES(${PROJECT_NAME} ${module} deps)

        LIST(APPEND mkdeps "lib${libname}")
        LIST(APPEND deps_cp "source/${PROJECT_NAME}/${module}")
        LIST(APPEND deps_cp "source/${PROJECT_NAME}/ports/${LEXBOR_OS_PORT_NAME}/config.cmake")

        set(port_path "source/${PROJECT_NAME}/ports/${LEXBOR_OS_PORT_NAME}/${PROJECT_NAME}/${module}")

        IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${port_path}")
            LIST(APPEND deps_cp "${port_path}")
        ENDIF()

        IF(NOT ${deps} STREQUAL "")
            string(REPLACE " " ";" deps ${deps})

            FOREACH(dep ${deps})
                LIST(APPEND deps_cp "source/${PROJECT_NAME}/${dep}")

                set(port_path "source/${PROJECT_NAME}/ports/${LEXBOR_OS_PORT_NAME}/${PROJECT_NAME}/${dep}")

                IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${port_path}")
                    LIST(APPEND deps_cp "${port_path}")
                ENDIF()
            ENDFOREACH()
        ENDIF()

        LIST_JOIN(deps_cp " " deps_cp)

        STRING(REGEX REPLACE "%%MODULE%%" "${module}" module_out "${module_out}")
        STRING(REGEX REPLACE "%%LIBNAME%%" "${libname}" module_out "${module_out}")
        STRING(REGEX REPLACE "%%MODULE_VERSION%%" "${version_string}" module_out "${module_out}")
        STRING(REGEX REPLACE "%%FOR_COPY%%" "${deps_cp}" module_out "${module_out}")

        set(mkmodules "${mkmodules}\n# ${module}\n${module_out}")
    ENDFOREACH()

    # Create Makefile
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/Makefile.in" mkfile_in)

    LIST_JOIN(mkdeps " " mkdeps)
    STRING(REGEX REPLACE "%%MODULES_DEPS%%" "${mkdeps}" mkfile_in "${mkfile_in}")
    STRING(REGEX REPLACE "%%MODULES%%" "${mkmodules}" mkfile_in "${mkfile_in}")

    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/Makefile" "${mkfile_in}")

    unset(port_path)
    unset(mkdeps)
    unset(mkmodules)
    unset(deps_cp)
    unset(libname)
    unset(mkfile_in)
    unset(module_out)
    unset(mkmodule_in)
ENDMACRO()

MACRO(PACKAGE_DEB_CREATE_DEBIAN_MAIN arch codename curdate)
    set(debian_dir "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/debian/${PROJECT_NAME}")
    set(debian_in_dir "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/debian_main_in")

    message(STATUS "Create DEB files for \"${PROJECT_NAME}\": ${debian_dir}")

    set(deps "${LEXBOR_MODULES}")

    file(MAKE_DIRECTORY "${debian_dir}/source")

    # changelog
    file(READ "${debian_in_dir}/changelog" data)
    STRING(REGEX REPLACE "%%LIBNAME%%" "${PROJECT_NAME}" data "${data}")
    STRING(REGEX REPLACE "%%VERSION%%" "${LEXBOR_VERSION_STRING}" data "${data}")
    STRING(REGEX REPLACE "%%DISTRO%%" "${LEXBOR_MAKE_DISTRO_NUM}" data "${data}")
    STRING(REGEX REPLACE "%%CODENAME%%" "${codename}" data "${data}")
    STRING(REGEX REPLACE "%%DATE%%" "${curdate}" data "${data}")
    file(WRITE "${debian_dir}/changelog" "${data}")

    # compat
    file(COPY "${debian_in_dir}/compat" DESTINATION "${debian_dir}")

    # docs
    file(COPY "${debian_in_dir}/docs" DESTINATION "${debian_dir}")

    # rules
    file(COPY "${debian_in_dir}/rules" DESTINATION "${debian_dir}")

    # source/format
    file(COPY "${debian_in_dir}/source/format" DESTINATION "${debian_dir}/source")

    # control
    file(READ "${debian_in_dir}/control" data)
    STRING(REGEX REPLACE "%%VERSION_DISTRO%%" "${LEXBOR_VERSION_STRING}-${LEXBOR_MAKE_DISTRO_NUM}~${codename}" data "${data}")

    # control -- sort modules
    set(sorted_modules ${deps})
    LIST(SORT sorted_modules)

    LIST_TO_COLUMN("${sorted_modules}" "79" ", " modules_names)

    IF(NOT ${modules_names} STREQUAL "")
        STRING(REGEX REPLACE "\n" "\n " modules_names ${modules_names})
        STRING(STRIP ${modules_names} modules_names)
    ENDIF()

    # control -- replace and save
    STRING(REGEX REPLACE "%%NAME%%" "${module}" data "${data}")
    STRING(REGEX REPLACE "%%LIBNAME%%" "${PROJECT_NAME}" data "${data}")
    STRING(REGEX REPLACE "%%MODULES_NAMES%%" "${modules_names}" data ${data})

    file(WRITE "${debian_dir}/control" "${data}")

    # copyright
    file(READ "${debian_in_dir}/copyright" data)

    set(files "")
    string(TIMESTAMP year "%Y")

    SUBDIRLIST("${CMAKE_CURRENT_SOURCE_DIR}/source" "source/" "/*" dirs_list)

    LIST(APPEND files ${dirs_list})
    LIST_JOIN(files "\n " files)

    STRING(REGEX REPLACE "%%YEAR%%" "${year}" data "${data}")
    STRING(REGEX REPLACE "%%FILES%%" "${files}" data "${data}")

    file(WRITE "${debian_dir}/copyright" "${data}")

    # dirs and install
    CREATE_DEB_DIRS(TRUE "" ${PROJECT_NAME} "${arch}" "${debian_in_dir}" "${debian_dir}")
ENDMACRO()

MACRO(PACKAGE_DEB_CREATE_DEBIAN arch codename curdate)
    PACKAGE_DEB_CREATE_DEBIAN_MAIN(${arch} "${codename}" "${curdate}")

    FOREACH(module ${LEXBOR_MODULES})
        set(libname "${PROJECT_NAME}-${module}")
        set(debian_dir "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/debian/${module}")
        set(debian_in_dir "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb/debian_in")

        message(STATUS "Create DEB files for \"${module}\": ${debian_dir}")

        INCLUDE_MODULE_CONFIG(${PROJECT_NAME} ${module} "${LEXBOR_SOURCE_LEXBOR}/${module}")

        GET_MODULE_VERSION(major minor patch version_string
                           "${LEXBOR_SOURCE}" ${PROJECT_NAME} ${module})
        GET_MODULE_DEPENDENCIES(${PROJECT_NAME} ${module} deps)

        IF(NOT "${deps}" STREQUAL "")
            string(REPLACE " " ";" deps ${deps})
        ENDIF()

        file(MAKE_DIRECTORY "${debian_dir}/source")

        # changelog
        file(READ "${debian_in_dir}/changelog" data)
        STRING(REGEX REPLACE "%%LIBNAME%%" "${libname}" data "${data}")
        STRING(REGEX REPLACE "%%VERSION%%" "${version_string}" data "${data}")
        STRING(REGEX REPLACE "%%DISTRO%%" "${LEXBOR_MAKE_DISTRO_NUM}" data "${data}")
        STRING(REGEX REPLACE "%%CODENAME%%" "${codename}" data "${data}")
        STRING(REGEX REPLACE "%%DATE%%" "${curdate}" data "${data}")
        file(WRITE "${debian_dir}/changelog" "${data}")

        # compat
        file(COPY "${debian_in_dir}/compat" DESTINATION "${debian_dir}")

        # docs
        file(COPY "${debian_in_dir}/docs" DESTINATION "${debian_dir}")

        # rules
        file(COPY "${debian_in_dir}/rules" DESTINATION "${debian_dir}")

        # source/format
        file(COPY "${debian_in_dir}/source/format" DESTINATION "${debian_dir}/source")

        # control
        file(READ "${debian_in_dir}/control" data)

        # control -- Depends
        set(version_distro "${version_string}-${LEXBOR_MAKE_DISTRO_NUM}~${codename}")
    
        set(requires "\${misc:Depends}, \${shlibs:Depends}")
        set(requires_devel "\${misc:Depends}")
        set(requires_devel "lib${libname} (= ${version_distro})")

        FOREACH(dep ${deps})
            IF("${dep}" STREQUAL "")
                CONTINUE()
            ENDIF()

            GET_MODULE_VERSION(dep_major dep_minor dep_patch dep_version_string
                               "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${dep})

            set(dep_version_distro "${dep_version_string}-${LEXBOR_MAKE_DISTRO_NUM}~${codename}")

            set(dep_libname "${PROJECT_NAME}-${dep}")
            LIST(APPEND requires "lib${dep_libname} (= ${dep_version_distro})")
            LIST(APPEND requires_devel "lib${dep_libname}-dev (= ${dep_version_distro})")
        ENDFOREACH()

        LIST_JOIN(requires ",\n         " requires)
        LIST_JOIN(requires_devel ",\n         " requires_devel)
    
        set(requires "Depends: ${requires}\n")
        set(requires_devel "Depends: ${requires_devel}\n")

        # control -- description
        GET_MODULE_DESCRIPTION("${PROJECT_NAME}" ${module} desc)

        IF(NOT "${desc}" STREQUAL "")
            STRING(REGEX REPLACE "\n" "\n " desc ${desc})
            STRING(STRIP ${desc} desc)
            set(desc " ${desc}")
        ENDIF()

        IF(NOT "${desc}" STREQUAL "")
            STRING(REGEX REPLACE "(^|\n) (\n|$)" "\\1 .\\2" desc ${desc})
        ENDIF()

        # control -- replace and save
        STRING(REGEX REPLACE "%%NAME%%" "${module}" data "${data}")
        STRING(REGEX REPLACE "%%LIBNAME%%" "${libname}" data "${data}")
        STRING(REGEX REPLACE "%%DEPENDS%%" "${requires}" data ${data})
        STRING(REGEX REPLACE "%%DEPENDS_DEVEL%%" "${requires_devel}" data ${data})
        STRING(REGEX REPLACE "%%DESCRIPTION%%" "${desc}" data ${data})

        file(WRITE "${debian_dir}/control" "${data}")

        # copyright
        file(READ "${debian_in_dir}/copyright" data)

        set(files "")
        set(deps_with "${deps};${module}")
        string(TIMESTAMP year "%Y")

        FOREACH(dep ${deps_with})
            IF("${dep}" STREQUAL "")
                CONTINUE()
            ENDIF()

            SUBDIRLIST("${CMAKE_CURRENT_SOURCE_DIR}/source/${PROJECT_NAME}/${dep}"
                       "source/${PROJECT_NAME}/${dep}/" "/*" dirs_list)
            LIST(APPEND files "source/${PROJECT_NAME}/${dep}/*")
            LIST(APPEND files ${dirs_list})
        ENDFOREACH()

        LIST_JOIN(files "\n " files)

        STRING(REGEX REPLACE "%%NAME%%" "${module}" data "${data}")
        STRING(REGEX REPLACE "%%YEAR%%" "${year}" data "${data}")
        STRING(REGEX REPLACE "%%FILES%%" "${files}" data "${data}")

        file(WRITE "${debian_dir}/copyright" "${data}")

        # dirs and install
        CREATE_DEB_DIRS(TRUE ${module} ${libname} "${arch}" "${debian_in_dir}"
                        "${debian_dir}")
    ENDFOREACH()

    unset(requires)
    unset(requires_devel)
    unset(dirs_list)
    unset(files)
    unset(year)
    unset(deps)
    unset(data)
    unset(debian_in_dir)
    unset(debian_dir)
    unset(dep_libname)
    unset(libname)
    unset(arch)
ENDMACRO()

MACRO(CREATE_DEB_FILES)
    execute_process(COMMAND "dpkg-architecture" "-qDEB_HOST_MULTIARCH"
                    OUTPUT_VARIABLE arch)

    IF("${arch}" STREQUAL "")
        message(FATAL_ERROR "Failed to get current architecture")
    ENDIF()

    execute_process(COMMAND "lsb_release" "-cs" OUTPUT_VARIABLE codename)

    IF("${codename}" STREQUAL "")
        message(FATAL_ERROR "Failed to get current codename")
    ENDIF()

    execute_process(COMMAND "date" "+%a, %d %b %Y %T +0300"
                    OUTPUT_VARIABLE curdate)

    IF("${curdate}" STREQUAL "")
        message(FATAL_ERROR "Failed to get current date")
    ENDIF()

    STRING(STRIP ${arch} arch)
    STRING(STRIP ${codename} codename)
    STRING(STRIP ${curdate} curdate)

    PACKAGE_DEB_CREATE_MAKEFILES(${arch})
    PACKAGE_DEB_CREATE_DEBIAN(${arch} "${codename}" "${curdate}")

    unset(arch)
    unset(codename)
    unset(curdate)
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
