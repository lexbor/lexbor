/*
 * Copyright (C) 2025-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/html/interfaces/style_element.h"
#include "lexbor/style/style.h"


lxb_status_t
lxb_html_style_element_parse(lxb_html_style_element_t *element)
{
    lxb_status_t status;
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

    stylesheet = lxb_css_stylesheet_create(css->memory);
    if (stylesheet == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    status = lxb_css_stylesheet_parse(stylesheet, css->parser,
                                      str->data, str->length);
    if (status != LXB_STATUS_OK) {
        (void) lxb_css_stylesheet_destroy(stylesheet, false);
        return status;
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
lxb_html_style_element_pop_open_elements(lxb_dom_node_t *node)
{
    lxb_status_t status;
    lxb_html_style_element_t *style;
    lxb_html_document_t *document;

    /* Only HTML namespaces. */

    style = lxb_html_interface_style(node);
    document = lxb_html_interface_document(node->owner_document);

    status = lxb_html_style_element_parse(style);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_html_document_stylesheet_add(document, style->stylesheet);
}

lxb_status_t
lxb_html_style_element_insert_steps(lxb_dom_node_t *inserted_node)
{
    lxb_status_t status;
    lxb_html_style_element_t *style;

    if (inserted_node->ns == LXB_NS_HTML) {
        style = lxb_html_interface_style(inserted_node);

        status = lxb_html_style_element_parse(style);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        if (style->stylesheet == NULL) {
            return LXB_STATUS_OK;
        }

        status = lxb_dom_document_stylesheet_attach(inserted_node->owner_document,
                                                    style->stylesheet);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_style_html_element_inserted_steps(inserted_node);
}

lxb_status_t
lxb_html_style_element_remove_steps(lxb_dom_node_t *removed_node,
                                    lxb_dom_node_t *old_parent)
{
    lxb_status_t status;

    if (removed_node->ns == LXB_NS_HTML) {
        status = lxb_html_style_element_remove(lxb_html_interface_style(removed_node));
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_style_html_element_removed_steps(removed_node, old_parent);
}

lxb_status_t
lxb_html_style_element_destroy_steps(lxb_dom_node_t *node)
{
    return lxb_style_html_element_destroy_steps(node);
}
