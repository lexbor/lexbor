cmake_minimum_required(VERSION 3.0)

# Options:
#    PROJECT_NAME                   default: lexbor
#    LEXBOR_OPTIMIZATION_LEVEL      default: -O2
#    LEXBOR_C_FLAGS                 default: see this file
#    LEXBOR_WITHOUT_THREADS         default: ON; Not used now, for the future
#    LEXBOR_BUILD_SHARED            default: ON; Create shaded library
#    LEXBOR_BUILD_STATIC            default: ON; Create static library
#    LEXBOR_INSTALL_HEADERS         default: ON; The header files will be installed 
#                                    if set to ON
#    LEXBOR_BUILD_TESTS             default: OFF; Build tests

if(NOT DEFINED PROJECT_NAME)
set(PROJECT_NAME "lexbor")
endif()

project(${PROJECT_NAME})

message(STATUS "Project name: ${PROJECT_NAME}")

################
## Options
#########################
option(LEXBOR_WITHOUT_THREADS "Build without Threads" ON)
option(LEXBOR_BUILD_SHARED "Build shared library" ON)
option(LEXBOR_BUILD_STATIC "Build static library" ON)
option(LEXBOR_INSTALL_HEADERS "Install header files" ON)
option(LEXBOR_BUILD_TESTS "Build tests" OFF)

################
## Version and path
#########################
set(LEXBOR_BASE "core")
set(LEXBOR_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(LEXBOR_SOURCE_LEXBOR "${LEXBOR_SOURCE}/lexbor")
set(LEXBOR_DIR_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(LEXBOR_DIR_HEADER "${LEXBOR_SOURCE}")
set(LEXBOR_DIR_API_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LEXBOR_DIR_API_HEADER_LEXBOR "${CMAKE_CURRENT_SOURCE_DIR}/include/lexbor")
set(LEXBOR_SOURCE_PORT_DIR "${LEXBOR_SOURCE}/ports")

set(LEXBOR_LIB_NAME "${PROJECT_NAME}")
set(LEXBOR_LIB_NAME_STATIC "${PROJECT_NAME}_static")

set(LEXBOR_INSTALL_DLL_EXE_DIR "bin")

################
## Project Version
#########################
set(LEXBOR_VERSION_PREFIX "LEXBOR_VERSION")
set(LEXBOR_VERSION_HEADER_FILE "${LEXBOR_DIR_API_HEADER_LEXBOR}/${LEXBOR_BASE}/base.h")

file(STRINGS ${LEXBOR_VERSION_HEADER_FILE} LEXBOR_VERSION_PARTS
REGEX "^#define[ \t]+${LEXBOR_VERSION_PREFIX}_(MAJOR|MINOR|PATCH)[ \t]+[0-9]+$")

list(GET LEXBOR_VERSION_PARTS 0 LEXBOR_VERSION_MAJOR_PART)
list(GET LEXBOR_VERSION_PARTS 1 LEXBOR_VERSION_MINOR_PART)
list(GET LEXBOR_VERSION_PARTS 2 LEXBOR_VERSION_PATCH_PART)

string(REGEX REPLACE "#define[ \t]+${LEXBOR_VERSION_PREFIX}_MAJOR[ \t]+([0-9]+).*" "\\1" LEXBOR_VERSION_MAJOR ${LEXBOR_VERSION_MAJOR_PART})
string(REGEX REPLACE "#define[ \t]+${LEXBOR_VERSION_PREFIX}_MINOR[ \t]+([0-9]+).*" "\\1" LEXBOR_VERSION_MINOR ${LEXBOR_VERSION_MINOR_PART})
string(REGEX REPLACE "#define[ \t]+${LEXBOR_VERSION_PREFIX}_PATCH[ \t]+([0-9]+).*" "\\1" LEXBOR_VERSION_PATCH ${LEXBOR_VERSION_PATCH_PART})

set(LEXBOR_VERSION_STRING "${LEXBOR_VERSION_MAJOR}.${LEXBOR_VERSION_MINOR}.${LEXBOR_VERSION_PATCH}")
message(STATUS "Project version: ${LEXBOR_VERSION_STRING}")

################
## lib param
#########################
set(LEXBOR_LIB_NAME "${PROJECT_NAME}")
set(LEXBOR_LIB_NAME_STATIC "${PROJECT_NAME}_static")

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
set(CMAKE_INSTALL_LIBDIR "lib")
endif()

################
## CMake Includes
#########################
include("${CMAKE_CURRENT_SOURCE_DIR}/config.cmake")

################
## Includes
#########################
include_directories(${LEXBOR_DIR_API_HEADER})
file(GLOB_RECURSE LEXBOR_HEADERS ${LEXBOR_DIR_API_HEADER}/*.h)

################
## Sources
#########################
GET_MODULES_LIST(LEXBOR_MODULES LEXBOR_MODULES_DIRS ${LEXBOR_SOURCE})
CREATE_MODULES_SOURCES(LEXBOR_MODULES_SOURCES_PATH "${LEXBOR_MODULES_DIRS}")

MODULES_PRINT("${LEXBOR_MODULES}")

file(GLOB_RECURSE LEXBOR_SOURCES ${LEXBOR_MODULES_SOURCES_PATH})

################
## Export LEXBOR_INCLUDE_DIRS and LEXBOR_LIBRARIES to global CACHE
#########################
set(LEXBOR_INCLUDES ${LEXBOR_DIR_API_HEADER} CACHE STRING "Include paths for ${PROJECT_NAME}")
set(LEXBOR_LIBRARIES ${LEXBOR_NAME} CACHE STRING "Libraries to link for ${PROJECT_NAME}")

################
## Create static/dynamic library
#########################
if(LEXBOR_BUILD_SHARED)
    add_library(${LEXBOR_LIB_NAME} SHARED ${LEXBOR_SOURCES})
    target_link_libraries(${LEXBOR_LIB_NAME} ${CMAKE_THREAD_LIBS_INIT})
    set_target_properties(${LEXBOR_LIB_NAME} PROPERTIES OUTPUT_NAME ${LEXBOR_LIB_NAME})
    set_target_properties(${LEXBOR_LIB_NAME} PROPERTIES VERSION ${LEXBOR_VERSION_STRING} SOVERSION ${LEXBOR_VERSION_MAJOR})
endif()

if(LEXBOR_BUILD_STATIC)
    add_library(${LEXBOR_LIB_NAME_STATIC} STATIC ${LEXBOR_SOURCES})
    target_link_libraries(${LEXBOR_LIB_NAME_STATIC} ${CMAKE_THREAD_LIBS_INIT})
    set_target_properties(${LEXBOR_LIB_NAME_STATIC} PROPERTIES OUTPUT_NAME ${LEXBOR_LIB_NAME_STATIC})
    set_target_properties(${LEXBOR_LIB_NAME_STATIC} PROPERTIES VERSION ${LEXBOR_VERSION_STRING} SOVERSION ${LEXBOR_VERSION_MAJOR})
endif()

################
## install
#########################
if(LEXBOR_INSTALL_HEADERS)
    file(GLOB LEXBOR_HEADER_DIRS "${LEXBOR_DIR_API_HEADER}/*")
    foreach(ITEM ${LEXBOR_HEADER_DIRS})                                                                                                                                                
        if(IS_DIRECTORY "${ITEM}")
            install(DIRECTORY "${ITEM}" DESTINATION "include" FILES_MATCHING PATTERN "*.h")
        endif()
    endforeach()
endif()

if(LEXBOR_BUILD_SHARED)
    install(TARGETS ${LEXBOR_LIB_NAME}

    RUNTIME DESTINATION "${LEXBOR_INSTALL_DLL_EXE_DIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

if(LEXBOR_BUILD_STATIC)
    install(TARGETS ${LEXBOR_LIB_NAME_STATIC}

    RUNTIME DESTINATION "${LEXBOR_INSTALL_DLL_EXE_DIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

################
## Tests
#########################
if(LEXBOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

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
