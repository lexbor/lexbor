/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/html/interfaces/document.h"
#include "lexbor/style/style.h"


lxb_status_t
lxb_html_document_script_cb(lxb_html_tree_t *tree, lxb_dom_node_t *node)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_document_style_cb(lxb_html_tree_t *tree, lxb_dom_node_t *node)
{
    lxb_status_t status;
    lxb_html_style_element_t *style;

    style = lxb_html_interface_style(node);

    status = lxb_html_style_element_parse(style);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_html_document_stylesheet_add(tree->document, style->stylesheet);
}

lxb_status_t
lxb_html_document_done_cb(lxb_html_document_t *document)
{
    size_t i, length;
    lxb_status_t status;
    lxb_css_stylesheet_t *sst;
    lxb_dom_document_t *dom_doc;

    dom_doc = &document->dom_document;

    if (dom_doc->css == NULL) {
        return LXB_STATUS_OK;
    }

    length = lexbor_array_length(dom_doc->css->stylesheets);

    for (i = 0; i < length; i++) {
        sst = lexbor_array_get(dom_doc->css->stylesheets, i);

        status = lxb_dom_document_stylesheet_apply(dom_doc, sst);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}
