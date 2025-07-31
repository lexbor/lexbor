cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE BENCHMARKS_LEXBOR_SELECTORS_SOURCES "*.c")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_selectors_" "${BENCHMARKS_LEXBOR_SELECTORS_SOURCES}" ${BENCHMARKS_DEPS_LIB_NAMES})
