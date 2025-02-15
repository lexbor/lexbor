/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_HTML_DOCUMENT_H
#define LEXBOR_STYLE_HTML_DOCUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/style/dom/interfaces/document.h"


LXB_API lxb_status_t
lxb_html_document_style_cb(lxb_html_tree_t *tree, lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_html_document_script_cb(lxb_html_tree_t *tree, lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_html_document_done_cb(lxb_html_document_t *document);


/*
 * Callbacks
 */
static const lxb_html_document_parse_cb_t lxb_html_parse_cb = {
    .script = lxb_html_document_script_cb,
    .style = lxb_html_document_style_cb
};


/*
 * Inline functions
 */
lxb_inline lxb_status_t
lxb_html_document_css_init(lxb_html_document_t *document, bool init_events)
{
    lxb_html_document_parse_cb_set(document, &lxb_html_parse_cb);
    lxb_html_document_done_set(document, lxb_html_document_done_cb);

    return lxb_dom_document_css_init(lxb_dom_interface_document(document),
                                     init_events);
}

lxb_inline void
lxb_html_document_css_clean(lxb_dom_document_t *document)
{
    lxb_dom_document_css_clean(lxb_dom_interface_document(document));
}

lxb_inline void
lxb_html_document_css_destroy(lxb_html_document_t *document)
{
    lxb_html_document_parse_cb_set(document, NULL);
    lxb_html_document_done_set(document, NULL);
    lxb_dom_document_css_destroy(lxb_dom_interface_document(document));
}

lxb_inline lxb_status_t
lxb_html_document_css_customs_init(lxb_html_document_t *document)
{
    return lxb_dom_document_css_customs_init(lxb_dom_interface_document(document));
}

lxb_inline void
lxb_html_document_css_customs_destroy(lxb_html_document_t *document)
{
    lxb_dom_document_css_customs_destroy(lxb_dom_interface_document(document));
}

lxb_inline uintptr_t
lxb_html_document_css_customs_find_id(const lxb_html_document_t *document,
                                      const lxb_char_t *key, size_t length)
{
    return lxb_dom_document_css_customs_find_id(lxb_dom_interface_document(document),
                                                key, length);
}

lxb_inline uintptr_t
lxb_html_document_css_customs_id(lxb_html_document_t *document,
                                 const lxb_char_t *key, size_t length)
{
    return lxb_dom_document_css_customs_id(lxb_dom_interface_document(document),
                                           key, length);
}

lxb_inline lxb_status_t
lxb_html_document_stylesheet_attach(lxb_html_document_t *document,
                                    lxb_css_stylesheet_t *sst)
{
    return lxb_dom_document_stylesheet_attach(lxb_dom_interface_document(document),
                                              sst);
}

lxb_inline lxb_status_t
lxb_html_document_stylesheet_apply(lxb_html_document_t *document,
                                   lxb_css_stylesheet_t *sst)
{
    return lxb_dom_document_stylesheet_apply(lxb_dom_interface_document(document),
                                             sst);
}

lxb_inline lxb_status_t
lxb_html_document_stylesheet_add(lxb_html_document_t *document,
                                 lxb_css_stylesheet_t *sst)
{
    return lxb_dom_document_stylesheet_add(lxb_dom_interface_document(document),
                                           sst);
}

lxb_inline lxb_status_t
lxb_html_document_stylesheet_remove(lxb_html_document_t *document,
                                          lxb_css_stylesheet_t *sst)
{
    return lxb_dom_document_stylesheet_remove(lxb_dom_interface_document(document),
                                              sst);
}

lxb_inline lxb_status_t
lxb_html_document_element_styles_attach(lxb_html_element_t *element)
{
    return lxb_dom_document_element_styles_attach(lxb_dom_interface_element(element));
}

lxb_inline void
lxb_html_document_stylesheet_destroy_all(lxb_html_document_t *document,
                                               bool destroy_memory)
{
    lxb_dom_document_stylesheet_destroy_all(lxb_dom_interface_document(document),
                                            destroy_memory);
}

lxb_inline lxb_status_t
lxb_html_document_style_attach(lxb_html_document_t *document,
                               lxb_css_rule_style_t *style)
{
    return lxb_dom_document_style_attach(lxb_dom_interface_document(document),
                                         style);
}

lxb_inline lxb_status_t
lxb_html_document_style_remove(lxb_html_document_t *document,
                               lxb_css_rule_style_t *style)
{
    return lxb_dom_document_style_remove(lxb_dom_interface_document(document),
                                         style);
}

lxb_inline lxb_status_t
lxb_html_document_style_attach_by_element(lxb_html_document_t *document,
                                          lxb_dom_element_t *element,
                                          lxb_css_rule_style_t *style)
{
    return lxb_dom_document_style_attach_by_element(lxb_dom_interface_document(document),
                                                    element, style);
}

lxb_inline lxb_status_t
lxb_html_document_apply_stylesheets(lxb_html_document_t *document)
{
    return lxb_dom_document_apply_stylesheets(lxb_dom_interface_document(document));
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_HTML_DOCUMENT_H */
