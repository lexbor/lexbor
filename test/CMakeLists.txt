cmake_minimum_required(VERSION 2.8)

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
FIND_AND_APPEND_SUB_DIRS("lexbor" OFF)
