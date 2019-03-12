cmake_minimum_required(VERSION 3.0)

project(examples_lexbor_html)

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_HTML_SOURCES "*.c")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_HTML_SOURCES}" ${LEXBOR_LIB_NAME})
