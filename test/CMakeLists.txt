cmake_minimum_required(VERSION 3.0)

project(lexbor_tests)

################
## Search and Includes
#########################
include_directories(".")

################
## Includes
#########################
include(unit/CMakeLists.txt)

################
## Subs
#########################
FIND_AND_APPEND_SUB_DIRS("lexbor")
