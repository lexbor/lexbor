/*
 * Copyright (C) 2019-2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_CSS_BASE_H
#define LEXBOR_CSS_BASE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "lexbor/core/base.h"


#define LXB_CSS_VERSION_MAJOR 0
#define LXB_CSS_VERSION_MINOR 3
#define LXB_CSS_VERSION_PATCH 0

#define LXB_CSS_VERSION_STRING                                                 \
    LEXBOR_STRINGIZE(LXB_CSS_VERSION_MAJOR) "."                                \
    LEXBOR_STRINGIZE(LXB_CSS_VERSION_MINOR) "."                                \
    LEXBOR_STRINGIZE(LXB_CSS_VERSION_PATCH)


typedef struct lxb_css_parser lxb_css_parser_t;
typedef struct lxb_css_parser_stack lxb_css_parser_stack_t;
typedef struct lxb_css_parser_error lxb_css_parser_error_t;

typedef struct lxb_css_syntax_tokenizer lxb_css_syntax_tokenizer_t;
typedef struct lxb_css_syntax_token lxb_css_syntax_token_t;

typedef bool
(*lxb_css_parser_state_f)(lxb_css_parser_t *parser,
                          lxb_css_syntax_token_t *token, void *ctx);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_CSS_BASE_H */
