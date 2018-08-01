/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_HTML_NS_H
#define LEXBOR_HTML_NS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/base.h"
#include "lexbor/html/ns_const.h"


typedef struct {
    const char       *name;
    const char       *name_lower;
    size_t           name_len;

    const char       *link;
    size_t           link_len;

    lxb_html_ns_id_t ns_id;
}
lxb_html_ns_data_t;


const lxb_html_ns_data_t *
lxb_html_ns_data_by_id(lxb_html_ns_id_t ns_id);

const lxb_html_ns_data_t *
lxb_html_ns_data_by_name(const lxb_char_t *name, size_t len);

const lxb_char_t *
lxb_html_ns_name_by_id(lxb_html_ns_id_t ns_id, size_t *len);

const lxb_char_t *
lxb_html_ns_lower_name_by_id(lxb_html_ns_id_t ns_id, size_t *len);

const lxb_char_t *
lxb_html_ns_link_by_id(lxb_html_ns_id_t ns_id, size_t *len);

lxb_html_ns_id_t
lxb_html_ns_id_by_name(const lxb_char_t *name, size_t len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_NS_H */
