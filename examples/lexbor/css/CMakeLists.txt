cmake_minimum_required(VERSION 3.0)

project(examples_lexbor_css)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_CSS_SOURCES "*.c")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_CSS_SOURCES}" ${LEXBOR_LIB_NAME})
