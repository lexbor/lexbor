cmake_minimum_required(VERSION 2.8.12...3.27)

################
## Sources
#########################
file(GLOB_RECURSE WASM_LEXBOR_ENGINE_SOURCES "*.c")

################
## Constants
#########################
set(WASM_CONSTANTS_HEADERS
    "lexbor/dom/interfaces/document.h:lxb_dom_document_opt:OPT"
    "lexbor/dom/interfaces/document.h:lxb_dom_document_cmode_t:CMODE"
    "lexbor/dom/interfaces/document.h:lxb_dom_document_dtype_t:DOCTYPE"
    "lexbor/html/serialize.h:lxb_html_serialize_opt:SERIALIZE"
)

################
## Create WASM modules
#########################
WASM_BUILD_LIST("wasm_" "${WASM_LEXBOR_ENGINE_SOURCES}")
