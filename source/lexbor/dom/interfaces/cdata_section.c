/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/dom/interfaces/cdata_section.h"
#include "lexbor/dom/interfaces/document.h"


lxb_dom_cdata_section_t *
lxb_dom_cdata_section_interface_create(lxb_dom_document_t *document)
{
    lxb_dom_cdata_section_t *element;

    element = lexbor_mraw_calloc(document->mraw,
                                 sizeof(lxb_dom_cdata_section_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = document;
    node->type = LXB_DOM_NODE_TYPE_CDATA_SECTION;

    return element;
}

lxb_dom_cdata_section_t *
lxb_dom_cdata_section_interface_destroy(lxb_dom_cdata_section_t *cdata_section)
{
    return lexbor_mraw_free(
        lxb_dom_interface_node(cdata_section)->owner_document->mraw,
        cdata_section);
}
