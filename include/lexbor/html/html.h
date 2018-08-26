/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_HTML_H
#define LEXBOR_HTML_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/html/base.h>
#include <lexbor/tag/const.h>
#include <lexbor/html/interfaces/document.h>


LXB_API lxb_dom_node_t *
lxb_html_create_node(lxb_html_document_t *document, lxb_tag_id_t tag_id,
                     lxb_ns_id_t ns);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_H */
