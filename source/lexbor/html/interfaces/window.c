/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/interfaces/window.h"
#include "lexbor/html/interfaces/document.h"


lxb_html_window_t *
lxb_html_window_create(lxb_html_document_t *document)
{
    lxb_html_window_t *element;

    element = lexbor_mraw_calloc(document->dom_document.mraw,
                                 sizeof(lxb_html_window_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = lxb_html_document_original_ref(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;
    node->ref_count = 1;

    return element;
}

lxb_html_window_t *
lxb_html_window_destroy(lxb_html_window_t *window)
{
    return lxb_html_interface_window(
        lxb_dom_node_interface_destroy(lxb_dom_interface_node(window)));
}
