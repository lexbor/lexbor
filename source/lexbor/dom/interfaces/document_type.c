/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/dom/interfaces/document_type.h"
#include "lexbor/dom/interfaces/document.h"


lxb_dom_document_type_t *
lxb_dom_document_type_interface_create(lxb_dom_document_t *document)
{
    lxb_dom_document_type_t *element;

    element = lexbor_mraw_calloc(document->mraw,
                                 sizeof(lxb_dom_document_type_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = document;
    node->type = LXB_DOM_NODE_TYPE_DOCUMENT_TYPE;

    return element;
}

lxb_dom_document_type_t *
lxb_dom_document_type_interface_destroy(lxb_dom_document_type_t *document_type)
{
    return lexbor_mraw_free(
        lxb_dom_interface_node(document_type)->owner_document->mraw,
        document_type);
}

/*
 * No inline functions for ABI.
 */
const lxb_char_t *
lxb_dom_document_type_name_noi(lxb_dom_document_type_t *doc_type, size_t *len)
{
    return lxb_dom_document_type_name(doc_type, len);
}

const lxb_char_t *
lxb_dom_document_type_public_id_noi(lxb_dom_document_type_t *doc_type,
                                    size_t *len)
{
    return lxb_dom_document_type_public_id(doc_type, len);
}

const lxb_char_t *
lxb_dom_document_type_system_id_noi(lxb_dom_document_type_t *doc_type,
                                    size_t *len)
{
    return lxb_dom_document_type_system_id(doc_type, len);
}
