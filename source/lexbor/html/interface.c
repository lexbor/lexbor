/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/core/mraw.h"

#include "lexbor/html/interface.h"
#include "lexbor/html/interfaces/document.h"

#include "lexbor/dom/interface.h"

#define LXB_HTML_INTERFACE_RES_CONSTRUCTORS
#define LXB_HTML_INTERFACE_RES_DESTRUCTOR
#include "lexbor/html/interface_res.h"


lxb_dom_interface_t *
lxb_html_interface_create(lxb_html_document_t *document, lxb_tag_id_t tag_id,
                          lxb_ns_id_t ns)
{
    lxb_dom_node_t *node;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (ns == LXB_NS_HTML) {
            lxb_html_unknown_element_t *unel;

            unel = lxb_html_unknown_element_interface_create(document);
            node = lxb_dom_interface_node(unel);
        }
        else if (ns == LXB_NS_SVG) {
            /* TODO: For this need implement SVGElement */
            lxb_dom_element_t *domel;

            domel = lxb_dom_element_interface_create(&document->dom_document);
            node = lxb_dom_interface_node(domel);
        }
        else {
            lxb_dom_element_t *domel;

            domel = lxb_dom_element_interface_create(&document->dom_document);
            node = lxb_dom_interface_node(domel);
        }
    }
    else {
        node = lxb_html_interface_res_constructors[tag_id][ns](document);
    }

    if (node == NULL) {
        return NULL;
    }

    node->tag_id = tag_id;
    node->ns = ns;

    return node;
}

lxb_dom_interface_t *
lxb_html_interface_destroy(lxb_dom_interface_t *interface)
{
    if (interface == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = interface;

    if (node->tag_id >= LXB_TAG__LAST_ENTRY) {
        if (node->ns == LXB_NS_HTML) {
            return lxb_html_unknown_element_interface_destroy(interface);
        }
        else if (node->ns == LXB_NS_SVG) {
            /* TODO: For this need implement SVGElement */
            return lxb_dom_element_interface_destroy(interface);
        }
        else {
            return lxb_dom_element_interface_destroy(interface);
        }
    }
    else {
        return lxb_html_interface_res_destructor[node->tag_id][node->ns](interface);
    }
}
