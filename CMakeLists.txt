cmake_minimum_required(VERSION 2.8)


# Options:
#    LEXBOR_OPTIMIZATION_LEVEL           default: -O2
#    LEXBOR_C_FLAGS                      default: see this file
#    LEXBOR_CXX_FLAGS                    default: see this file
#    LEXBOR_WITHOUT_THREADS              default: ON; Not used now, for the future
#    LEXBOR_BUILD_SHARED                 default: ON; Create shaded library
#    LEXBOR_BUILD_STATIC                 default: ON; Create static library
#    LEXBOR_BUILD_SEPARATELY             default: OFF; Build all modules separately.
#                                         Each module will have its own shared library
#    LEXBOR_BUILD_EXAMPLES               default: OFF; Build all examples
#    LEXBOR_BUILD_TESTS                  default: OFF; Build tests
#    LEXBOR_BUILD_TESTS_CPP              default: ON; Build C++ tests.
#                                         Used with LEXBOR_BUILD_TESTS
#    LEXBOR_BUILD_UTILS                  default: OFF; Build utils/helpers for project.
#    LEXBOR_BUILD_WITH_ASAN              default: OFF; Build with address sanitizer if possible
#    LEXBOR_BUILD_WITH_FUZZER            default: OFF; Build with fuzzer tests if possible
#    LEXBOR_INSTALL_HEADERS              default: ON; The header files will be installed
#                                         if set to ON
#    LEXBOR_MAKE_RPM_FILES               default: OFF; Create "spec" files in "packaging" direcotry
#                                         for create packages
#    LEXBOR_MAKE_DEB_FILES               default: OFF; Create "control" files in "packaging" direcotry
#                                         for create packages
#    LEXBOR_MAKE_DISTRO_NUM              default: 1; For packaging
#    LEXBOR_PRINT_MODULE_DEPENDENCIES    default: OFF; Prints dependencies between modules

# For build with ASAN use CMake flag:
#     -DLEXBOR_BUILD_WITH_ASAN=ON
#     or
#     -DCMAKE_C_FLAGS='-O0 -g -fsanitize=address'
#     or
#     -DLEXBOR_C_FLAGS='-O0 -g -fsanitize=address'


set(PROJECT_NAME "lexbor")

message(STATUS "Project name: ${PROJECT_NAME}")

################
## Options
#########################
option(LEXBOR_WITHOUT_THREADS "Build without Threads" ON)
option(LEXBOR_BUILD_SHARED "Build shared library" ON)
option(LEXBOR_BUILD_STATIC "Build static library" ON)
option(LEXBOR_BUILD_SEPARATELY "Build modules separately" OFF)
option(LEXBOR_BUILD_EXAMPLES "Build examples" OFF)
option(LEXBOR_BUILD_TESTS "Build tests" OFF)
option(LEXBOR_BUILD_TESTS_CPP "Build C++ tests" ON)
option(LEXBOR_BUILD_UTILS "Build utils" OFF)
option(LEXBOR_BUILD_WITH_ASAN "Build with address sanitizer" OFF)
option(LEXBOR_BUILD_WITH_FUZZER "Build with fuzzer" OFF)
option(LEXBOR_INSTALL_HEADERS "Install header files" ON)
option(LEXBOR_MAKE_PACKAGES_FILES "Create files for build packages" OFF)
option(LEXBOR_PRINT_MODULE_DEPENDENCIES "Prints dependencies" OFF)

IF(NOT LEXBOR_MAKE_DISTRO_NUM)
    set(LEXBOR_MAKE_DISTRO_NUM "1")
ENDIF()

################
## Version and path
#########################
set(LEXBOR_BASE "core")
set(LEXBOR_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source" CACHE STRING "")
set(LEXBOR_SOURCE_LEXBOR "${LEXBOR_SOURCE}/lexbor" CACHE STRING "")
set(LEXBOR_DIR_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib" CACHE STRING "")
set(LEXBOR_DIR_HEADER "${LEXBOR_SOURCE}" CACHE STRING "")
set(LEXBOR_DIR_HEADER_LEXBOR "${LEXBOR_SOURCE}/lexbor" CACHE STRING "")
set(LEXBOR_SOURCE_PORT_DIR "${LEXBOR_SOURCE_LEXBOR}/ports" CACHE STRING "")
set(LEXBOR_DIR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "")

set(LEXBOR_VERSION_FILEPATH "${CMAKE_CURRENT_SOURCE_DIR}/version" CACHE STRING "")

set(LEXBOR_INSTALL_DLL_EXE_DIR "bin")

################
## RPATH
#########################
include(GNUInstallDirs)

IF(APPLE)
    set(CMAKE_MACOSX_RPATH ON)

    set(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
ENDIF(APPLE)

################
## lib param
#########################
IF(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib")
ENDIF()

################
## CMake Includes
#########################
include("${CMAKE_CURRENT_SOURCE_DIR}/config.cmake")

################
## Project Version
#########################
GET_LEXBOR_VERSION(LEXBOR_VERSION_MAJOR LEXBOR_VERSION_MINOR
                   LEXBOR_VERSION_PATCH LEXBOR_VERSION_STRING)

message(STATUS "Lexbor version: ${LEXBOR_VERSION_STRING}")

IF(CMAKE_VERSION VERSION_LESS "3.0")
    PROJECT("${PROJECT_NAME}")
ELSE()
    cmake_policy(SET CMP0048 NEW)
    PROJECT("${PROJECT_NAME}" VERSION "${LEXBOR_VERSION_STRING}")
ENDIF()

################
## Features
#########################
include("${CMAKE_CURRENT_SOURCE_DIR}/feature.cmake")

################
## Includes
#########################
include_directories(${LEXBOR_DIR_HEADER})

################
## Export LEXBOR_INCLUDE_DIRS and LEXBOR_LIBRARIES to global CACHE
#########################
set(LEXBOR_INCLUDES ${LEXBOR_DIR_HEADER} CACHE STRING "Include paths for ${PROJECT_NAME}")
set(LEXBOR_LIBRARIES ${LEXBOR_LIB_NAME} CACHE STRING "Libraries to link for ${PROJECT_NAME}")

################
## Sources
#########################
GET_MODULES_LIST(LEXBOR_MODULES "${LEXBOR_SOURCE}")

################
## Create packages files
#########################
IF(LEXBOR_MAKE_RPM_FILES)
    CREATE_RPM_SPEC_FILE()
    RETURN()
ENDIF()

IF(LEXBOR_MAKE_DEB_FILES)
    CREATE_DEB_FILES()
    RETURN()
ENDIF()

################
## Build all modules in one liblexbor library
#########################
set(LEXBOR_LIB_NAME "${PROJECT_NAME}")
set(LEXBOR_LIB_NAME_STATIC "${LEXBOR_LIB_NAME}_static")

## Get all source files from modules
FOREACH(module ${LEXBOR_MODULES})
    GET_MODULE_RESURSES(headers sources "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})
    LIST(APPEND LEXBOR_SOURCES ${sources})
ENDFOREACH()

## First, need to add target for shared and static library
IF(LEXBOR_BUILD_SHARED)
    add_library(${LEXBOR_LIB_NAME} SHARED ${LEXBOR_SOURCES})
ENDIF()

IF(LEXBOR_BUILD_STATIC)
    add_library(${LEXBOR_LIB_NAME_STATIC} STATIC ${LEXBOR_SOURCES})
ENDIF()

## Load configurations from all modules. Print modules name and version.
IF(NOT LEXBOR_BUILD_SEPARATELY)
    FOREACH(module ${LEXBOR_MODULES})
        IF(LEXBOR_INSTALL_HEADERS)
            INSTALL_MODULE_HEADERS("${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})
        ENDIF()

        INCLUDE_MODULE_CONFIG(${PROJECT_NAME} ${module} "${LEXBOR_SOURCE_LEXBOR}/${module}")
        GET_MODULE_DEPENDENCIES(${PROJECT_NAME} ${module} module_deps)
        GET_MODULE_VERSION(major minor patch version_string
                           "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})

        MODULE_PRINT(${module} ${version_string} "${module_deps}")
    ENDFOREACH()
ENDIF()

## Create dynamic/static library
IF(LEXBOR_BUILD_SHARED)
    ADD_MODULE_LIBRARY(SHARED ${LEXBOR_LIB_NAME} ${LEXBOR_VERSION_STRING}
                       ${LEXBOR_VERSION_MAJOR})
ENDIF()

IF(LEXBOR_BUILD_STATIC)
    ADD_MODULE_LIBRARY(STATIC ${LEXBOR_LIB_NAME_STATIC} ${LEXBOR_VERSION_STRING}
                       ${LEXBOR_VERSION_MAJOR})
ENDIF()

################
## Build separately modules in liblexbor-<module-name>
#########################
IF(LEXBOR_BUILD_SEPARATELY)
    FOREACH(module ${LEXBOR_MODULES})
        set(lexbor_deps "${lexbor_deps} ${module}")
        set(libname "${PROJECT_NAME}-${module}")

        GET_MODULE_VERSION(major minor patch version_string
                           "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})

        GET_MODULE_RESURSES(headers sources "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})

        IF(LEXBOR_BUILD_SHARED)
            add_library(${libname} SHARED ${sources})
            ADD_MODULE_LIBRARY(SHARED ${libname} ${version_string} ${major})
        ENDIF()

        IF(LEXBOR_BUILD_STATIC)
            add_library("${libname}_static" STATIC ${sources})
            ADD_MODULE_LIBRARY(STATIC "${libname}_static" ${version_string} ${major})
        ENDIF()

        IF(LEXBOR_INSTALL_HEADERS)
            INSTALL_MODULE_HEADERS("${LEXBOR_SOURCE}" "${PROJECT_NAME}" "${module}")
        ENDIF()

        INCLUDE_MODULE_CONFIG(${PROJECT_NAME} ${module} "${LEXBOR_SOURCE_LEXBOR}/${module}")
        GET_MODULE_DEPENDENCIES(${PROJECT_NAME} ${module} module_deps)

        IF(LEXBOR_BUILD_SHARED)
            SET_MODULE_LIB_DEPENDENCIES(${libname} "${module_deps}" "")
        ENDIF()

        IF(LEXBOR_BUILD_STATIC)
            SET_MODULE_LIB_DEPENDENCIES("${libname}_static" "${module_deps}" "_static")
        ENDIF()

        MODULE_PRINT(${module} ${version_string} "${module_deps}")
    ENDFOREACH()
ENDIF()

################
## Sets C_FLAGS
#########################
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LEXBOR_OPTIMIZATION_LEVEL} ${LEXBOR_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LEXBOR_OPTIMIZATION_LEVEL} ${LEXBOR_CXX_FLAGS}")
message(STATUS "CFLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "CXXFLAGS: ${CMAKE_CXX_FLAGS}")

################
## Check features
#########################
FEATURE_CHECK_ASAN(LEXBOR_HAVE_ASAN)
IF(${LEXBOR_HAVE_ASAN})
    add_definitions(-DLEXBOR_HAVE_ADDRESS_SANITIZER)
ENDIF()

FEATURE_CHECK_FUZZER(LEXBOR_HAVE_FUZZER)
IF(${LEXBOR_HAVE_FUZZER})
    add_definitions(-DLEXBOR_HAVE_FUZZER)
ENDIF()

################
## Tests
#########################
IF(LEXBOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
ENDIF()

################
## Examples
#########################
IF(LEXBOR_BUILD_EXAMPLES)
    add_subdirectory(examples)
ENDIF()

################
## Utils
#########################
IF(LEXBOR_BUILD_UTILS)
    add_subdirectory(utils)
ENDIF()

################
## Build an RPM.
#########################
set(CPACK_PACKAGE_VERSION ${LEXBOR_VERSION_STRING})

if(LEXBOR_PLATFORM STREQUAL "Debian")
  set(CPACK_GENERATOR "DEB")
elseif(LEXBOR_PLATFORM STREQUAL "Redhat")
  set(CPACK_GENERATOR "RPM")
elseif(LEXBOR_PLATFORM STREQUAL "SuSe")
  set(CPACK_GENERATOR "RPM")
else()
  set(CPACK_GENERATOR "TGZ")
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "HTML Renderer")
set(CPACK_PACKAGE_CONTACT "Alexander Borisov")
set(CPACK_PACKAGE_VENDOR "Alexander Borisov")

if(32BIT)
  set(CPACK_RPM_PACKAGE_ARCHITECTURE i686)
else()
  set(CPACK_RPM_PACKAGE_ARCHITECTURE x86_64)
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CPACK_RPM_PACKAGE_ARCHITECTURE}")

include(CPack)
