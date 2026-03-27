cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Sources
#########################
file(GLOB_RECURSE WASM_LEXBOR_HTML_SOURCES "*.c")

################
## Create WASM modules
#########################
WASM_BUILD_LIST("wasm_" "${WASM_LEXBOR_HTML_SOURCES}")
