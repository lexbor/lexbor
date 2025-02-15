/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/html/interfaces/style_element.h"
#include "lexbor/style/style.h"


lxb_status_t
lxb_html_style_element_parse(lxb_html_style_element_t *element)
{
    lexbor_str_t *str;
    lxb_dom_text_t *text;
    lxb_dom_node_t *node;
    lxb_css_stylesheet_t *stylesheet;

    lxb_dom_document_t *doc = lxb_dom_interface_node(element)->owner_document;
    lxb_dom_document_css_t *css = doc->css;

    node = lxb_dom_interface_node(element);

    if (node->first_child == NULL
        || node->first_child->local_name != LXB_TAG__TEXT
        || node->first_child != node->last_child)
    {
        return LXB_STATUS_OK;
    }

    text = lxb_dom_interface_text(lxb_dom_interface_node(element)->first_child);
    str = &text->char_data.data;

    stylesheet = lxb_css_stylesheet_parse(css->parser, str->data, str->length);
    if (stylesheet == NULL) {
        return css->parser->status;
    }

    stylesheet->element = element;

    element->stylesheet = stylesheet;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_style_element_remove(lxb_html_style_element_t *element)
{
    lxb_dom_document_t *doc = lxb_dom_interface_node(element)->owner_document;

    lxb_dom_document_stylesheet_remove(doc, element->stylesheet);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_style_element_cb(lxb_html_style_element_t *element)
{
    lxb_status_t status;
    lxb_dom_document_t *doc;

    status = lxb_html_style_element_parse(element);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    doc = lxb_dom_interface_node(element)->owner_document;

    return lxb_dom_document_stylesheet_add(doc, element->stylesheet);
}
