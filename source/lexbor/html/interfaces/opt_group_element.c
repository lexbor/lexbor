/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/interfaces/opt_group_element.h"
#include "lexbor/html/interfaces/document.h"


lxb_html_opt_group_element_t *
lxb_html_opt_group_element_create(lxb_html_document_t *document)
{
    lxb_html_opt_group_element_t *element;

    element = lexbor_mraw_calloc(document->mem->mraw,
                                 sizeof(lxb_html_opt_group_element_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = lxb_dom_interface_document(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    return element;
}

lxb_html_opt_group_element_t *
lxb_html_opt_group_element_destroy(lxb_html_opt_group_element_t *opt_group_element)
{
    return lexbor_mraw_free(
        lxb_dom_interface_node(opt_group_element)->owner_document->mraw,
        opt_group_element);
}
