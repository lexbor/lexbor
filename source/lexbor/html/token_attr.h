/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_TOKEN_ATTR_H
#define LEXBOR_HTML_TOKEN_ATTR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/in.h"
#include "lexbor/core/str.h"
#include "lexbor/core/dobject.h"

#include "lexbor/html/base.h"
#include "lexbor/html/parser_char.h"


typedef struct lxb_html_token_attr lxb_html_token_attr_t;
typedef int lxb_html_token_attr_type_t;

enum lxb_html_token_attr_type {
    LXB_HTML_TOKEN_ATTR_TYPE_UNDEF      = 0x0000,
    LXB_HTML_TOKEN_ATTR_TYPE_NAME_NULL  = 0x0001,
    LXB_HTML_TOKEN_ATTR_TYPE_VALUE_NULL = 0x0002
};

struct lxb_html_token_attr {
    const lxb_char_t           *name_begin;
    const lxb_char_t           *name_end;

    const lxb_char_t           *value_begin;
    const lxb_char_t           *value_end;

    lexbor_in_node_t           *in_name;
    lexbor_in_node_t           *in_value;

    lxb_html_token_attr_t      *next;
    lxb_html_token_attr_t      *prev;

    lxb_html_token_attr_type_t type;
};


LXB_API lxb_html_token_attr_t *
lxb_html_token_attr_create(lexbor_dobject_t *dobj);

LXB_API void
lxb_html_token_attr_clean(lxb_html_token_attr_t *attr);

LXB_API lxb_html_token_attr_t *
lxb_html_token_attr_destroy(lxb_html_token_attr_t *attr,
                            lexbor_dobject_t *dobj);


LXB_API lxb_status_t
lxb_html_token_attr_make_name(lxb_html_token_attr_t *attr, lexbor_str_t *str,
                              lexbor_mraw_t *mraw);

LXB_API lxb_status_t
lxb_html_token_attr_make_value(lxb_html_token_attr_t *attr, lexbor_str_t *str,
                               lexbor_mraw_t *mraw);

LXB_API lxb_status_t
lxb_html_token_attr_parse(lxb_html_token_attr_t *attr,
                          lxb_html_parser_char_t *pc, lexbor_str_t *name,
                          lexbor_str_t *value, lexbor_mraw_t *mraw);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_TOKEN_ATTR_H */
