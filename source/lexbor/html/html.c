/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/html.h"
#include "lexbor/html/tree.h"
#include "lexbor/html/interfaces/unknown_element.h"


lxb_dom_node_t *
lxb_html_create_node(lxb_html_document_t *document, lxb_html_tag_id_t tag_id,
                     lxb_html_ns_id_t ns)
{
    const lxb_html_tag_data_t *tag_data;
    lxb_dom_node_t *node;

    tag_data = lxb_html_tag_data_by_id(document->mem->tag_heap_ref, tag_id);
    if (tag_data == NULL) {
        return NULL;
    }

    if (tag_data->interface[ns] == NULL) {
        if (ns == LXB_HTML_NS_HTML) {
            lxb_html_unknown_element_t *unel;

            unel = lxb_html_unknown_element_create(document);
            node = lxb_dom_interface_node(unel);
        }
        else if (ns == LXB_HTML_NS_SVG) {
            /* TODO: For this need implement SVGElement */
            lxb_dom_element_t *domel;

            domel = lxb_dom_element_create(&document->dom_document);
            node = lxb_dom_interface_node(domel);
        }
        else {
            lxb_dom_element_t *domel;

            domel = lxb_dom_element_create(&document->dom_document);
            node = lxb_dom_interface_node(domel);
        }
    }
    else {
        node = tag_data->interface[ns](document);
    }

    if (node == NULL) {
        return NULL;
    }

    node->tag_id = tag_id;
    node->ns = ns;

    return node;
}
