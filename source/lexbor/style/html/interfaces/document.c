/*
 * Copyright (C) 2025-2026 Alexander Borisov
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

/*
 * No inline functions for ABI.
 */
lxb_status_t
lxb_html_document_css_init_noi(lxb_html_document_t *document, bool init_events)
{
    return lxb_html_document_css_init(document, init_events);
}

void
lxb_html_document_css_clean_noi(lxb_dom_document_t *document)
{
    lxb_html_document_css_clean(document);
}

void
lxb_html_document_css_destroy_noi(lxb_html_document_t *document)
{
    lxb_html_document_css_destroy(document);
}

lxb_status_t
lxb_html_document_css_customs_init_noi(lxb_html_document_t *document)
{
    return lxb_html_document_css_customs_init(document);
}

void
lxb_html_document_css_customs_destroy_noi(lxb_html_document_t *document)
{
    lxb_html_document_css_customs_destroy(document);
}

uintptr_t
lxb_html_document_css_customs_find_id_noi(const lxb_html_document_t *document,
                                          const lxb_char_t *key, size_t length)
{
    return lxb_html_document_css_customs_find_id(document, key, length);
}

uintptr_t
lxb_html_document_css_customs_id_noi(lxb_html_document_t *document,
                                     const lxb_char_t *key, size_t length)
{
    return lxb_html_document_css_customs_id(document, key, length);
}

lxb_status_t
lxb_html_document_stylesheet_attach_noi(lxb_html_document_t *document,
                                        lxb_css_stylesheet_t *sst)
{
    return lxb_html_document_stylesheet_attach(document, sst);
}

lxb_status_t
lxb_html_document_stylesheet_apply_noi(lxb_html_document_t *document,
                                       lxb_css_stylesheet_t *sst)
{
    return lxb_html_document_stylesheet_apply(document, sst);
}

lxb_status_t
lxb_html_document_stylesheet_add_noi(lxb_html_document_t *document,
                                     lxb_css_stylesheet_t *sst)
{
    return lxb_html_document_stylesheet_add(document, sst);
}

lxb_status_t
lxb_html_document_stylesheet_remove_noi(lxb_html_document_t *document,
                                        lxb_css_stylesheet_t *sst)
{
    return lxb_html_document_stylesheet_remove(document, sst);
}

lxb_status_t
lxb_html_document_element_styles_attach_noi(lxb_html_element_t *element)
{
    return lxb_html_document_element_styles_attach(element);
}

void
lxb_html_document_stylesheet_destroy_all_noi(lxb_html_document_t *document,
                                             bool destroy_memory)
{
    lxb_html_document_stylesheet_destroy_all(document, destroy_memory);
}

lxb_status_t
lxb_html_document_style_attach_noi(lxb_html_document_t *document,
                                   lxb_css_rule_style_t *style)
{
    return lxb_html_document_style_attach(document, style);
}

lxb_status_t
lxb_html_document_style_remove_noi(lxb_html_document_t *document,
                                   lxb_css_rule_style_t *style)
{
    return lxb_html_document_style_remove(document, style);
}

lxb_status_t
lxb_html_document_style_attach_by_element_noi(lxb_html_document_t *document,
                                              lxb_dom_element_t *element,
                                              lxb_css_rule_style_t *style)
{
    return lxb_html_document_style_attach_by_element(document, element, style);
}
