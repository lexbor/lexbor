/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/interfaces/template_element.h"
#include "lexbor/html/interfaces/document.h"


lxb_html_template_element_t *
lxb_html_template_element_create(lxb_html_document_t *document)
{
    lxb_html_template_element_t *element;

    element = lexbor_mraw_calloc(document->mem->mraw,
                                 sizeof(lxb_html_template_element_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = lxb_dom_interface_document(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    element->content = lxb_dom_document_fragment_create(node->owner_document);
    if (element->content == NULL) {
        return lxb_html_template_element_destroy(element);
    }

    element->content->node.ns = LXB_HTML_NS_HTML;
    element->content->host = lxb_dom_interface_element(element);

    return element;
}

lxb_html_template_element_t *
lxb_html_template_element_destroy(lxb_html_template_element_t *template_element)
{
    lxb_dom_document_fragment_destroy(template_element->content);

    return lexbor_mraw_free(
        lxb_dom_interface_node(template_element)->owner_document->mraw,
        template_element);
}
