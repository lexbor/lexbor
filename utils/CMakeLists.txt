cmake_minimum_required(VERSION 2.8)

project(lexbor_utils)

################
## Search and Includes
#########################
include_directories(".")

################
## Subs
#########################
FIND_AND_APPEND_SUB_DIRS("lexbor" ON)
