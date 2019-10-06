/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
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


LXB_API lxb_dom_document_type_t *
lxb_dom_document_type_interface_create(lxb_dom_document_t *document);

LXB_API lxb_dom_document_type_t *
lxb_dom_document_type_interface_destroy(lxb_dom_document_type_t *document_type);


/*
 * Inline functions
 */
lxb_inline const lxb_char_t *
lxb_dom_document_type_name(lxb_dom_document_type_t *doc_type, size_t *len)
{
    if (len != NULL) {
        *len = doc_type->name.length;
    }

    return doc_type->name.data;
}

lxb_inline const lxb_char_t *
lxb_dom_document_type_public_id(lxb_dom_document_type_t *doc_type, size_t *len)
{
    if (len != NULL) {
        *len = doc_type->public_id.length;
    }

    return doc_type->public_id.data;
}

lxb_inline const lxb_char_t *
lxb_dom_document_type_system_id(lxb_dom_document_type_t *doc_type, size_t *len)
{
    if (len != NULL) {
        *len = doc_type->system_id.length;
    }

    return doc_type->system_id.data;
}

/*
 * No inline functions for ABI.
 */
const lxb_char_t *
lxb_dom_document_type_name_noi(lxb_dom_document_type_t *doc_type, size_t *len);

const lxb_char_t *
lxb_dom_document_type_public_id_noi(lxb_dom_document_type_t *doc_type,
                                    size_t *len);

const lxb_char_t *
lxb_dom_document_type_system_id_noi(lxb_dom_document_type_t *doc_type,
                                    size_t *len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_DOCUMENT_TYPE_H */
