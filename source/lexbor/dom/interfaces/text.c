/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/interfaces/text.h"
#include "lexbor/dom/interfaces/document.h"


lxb_dom_text_t *
lxb_dom_text_create(lxb_dom_document_t *document)
{
    lxb_dom_text_t *element;

    element = lexbor_mraw_calloc(document->mraw,
                                 sizeof(lxb_dom_text_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = document;
    node->type = LXB_DOM_NODE_TYPE_TEXT;

    return element;
}

lxb_dom_text_t *
lxb_dom_text_destroy(lxb_dom_text_t *text)
{
    return lexbor_mraw_free(
        lxb_dom_interface_node(text)->owner_document->mraw,
        text);
}
