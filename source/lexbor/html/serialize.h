/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_HTML_SERIALIZE_H
#define LEXBOR_HTML_SERIALIZE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/dom/interfaces/element.h"

#include "lexbor/html/base.h"

typedef enum {
    LXB_HTML_SERIALIZE_OPT_UNDEF           = 0x00,
    LXB_HTML_SERIALIZE_OPT_SKIP_WS_NODES   = 0x01,
    LXB_HTML_SERIALIZE_OPT_SKIP_COMMENT    = 0x02,
    LXB_HTML_SERIALIZE_OPT_RAW             = 0x04,
    LXB_HTML_SERIALIZE_OPT_WITHOUT_CLOSING = 0x08
}
lxb_html_serialize_opt_t;


typedef lxb_status_t
(*lxb_html_serialize_cb_f)(const lxb_char_t *data, size_t len, void *ctx);


lxb_status_t
lxb_html_serialize_cb(lxb_dom_node_t *node,
                      lxb_html_serialize_cb_f cb, void *ctx);

lxb_status_t
lxb_html_serialize_tree_cb(lxb_dom_node_t *node,
                           lxb_html_serialize_cb_f cb, void *ctx);

lxb_status_t
lxb_html_serialize_pretty_cb(lxb_dom_node_t *node,
                             lxb_html_serialize_opt_t opt, size_t indent,
                             lxb_html_serialize_cb_f cb, void *ctx);

lxb_status_t
lxb_html_serialize_pretty_tree_cb(lxb_dom_node_t *node,
                                  lxb_html_serialize_opt_t opt, size_t indent,
                                  lxb_html_serialize_cb_f cb, void *ctx);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_SERIALIZE_H */
