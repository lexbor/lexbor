cmake_minimum_required(VERSION 3.0)

project(lexbor_examples)

################
## Search and Includes
#########################
include_directories(".")

################
## Subs
#########################
FIND_AND_APPEND_SUB_DIRS("lexbor")
