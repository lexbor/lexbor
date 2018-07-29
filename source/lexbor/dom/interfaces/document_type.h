/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_DOM_DOCUMENT_TYPE_H
#define LEXBOR_DOM_DOCUMENT_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/str.h"

#include "lexbor/dom/interfaces/document.h"
#include "lexbor/dom/interfaces/node.h"


struct lxb_dom_document_type {
    lxb_dom_node_t node;

    lexbor_str_t   name;
    lexbor_str_t   public_id;
    lexbor_str_t   system_id;
};


lxb_dom_document_type_t *
lxb_dom_document_type_create(lxb_dom_document_t *document);

lxb_dom_document_type_t *
lxb_dom_document_type_destroy(lxb_dom_document_type_t *document_type);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_DOCUMENT_TYPE_H */
