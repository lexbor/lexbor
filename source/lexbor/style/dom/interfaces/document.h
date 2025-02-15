/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_DOM_DOCUMENT_H
#define LEXBOR_STYLE_DOM_DOCUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/style/base.h"


struct lxb_dom_document_css {
    lxb_css_memory_t    *memory;
    lxb_css_selectors_t *css_selectors;
    lxb_css_parser_t    *parser;
    lxb_selectors_t     *selectors;

    lexbor_avl_t        *styles;
    lexbor_array_t      *stylesheets;
    lexbor_dobject_t    *weak;

    lexbor_hash_t       *customs;
    uintptr_t           customs_id;
};


LXB_API lxb_status_t
lxb_dom_document_css_init(lxb_dom_document_t *document, bool init_events);

LXB_API void
lxb_dom_document_css_clean(lxb_dom_document_t *document);

LXB_API void
lxb_dom_document_css_destroy(lxb_dom_document_t *document);

LXB_API lxb_status_t
lxb_dom_document_css_customs_init(lxb_dom_document_t *document);

LXB_API void
lxb_dom_document_css_customs_destroy(lxb_dom_document_t *document);

LXB_API uintptr_t
lxb_dom_document_css_customs_find_id(const lxb_dom_document_t *document,
                                     const lxb_char_t *key, size_t length);

LXB_API uintptr_t
lxb_dom_document_css_customs_id(lxb_dom_document_t *document,
                                const lxb_char_t *key, size_t length);

LXB_API lxb_status_t
lxb_dom_document_stylesheet_attach(lxb_dom_document_t *document,
                                   lxb_css_stylesheet_t *sst);

LXB_API lxb_status_t
lxb_dom_document_stylesheet_apply(lxb_dom_document_t *document,
                                  lxb_css_stylesheet_t *sst);

LXB_API lxb_status_t
lxb_dom_document_stylesheet_add(lxb_dom_document_t *document,
                                lxb_css_stylesheet_t *sst);

LXB_API lxb_status_t
lxb_dom_document_stylesheet_remove(lxb_dom_document_t *document,
                                   lxb_css_stylesheet_t *sst);

LXB_API lxb_status_t
lxb_dom_document_element_styles_attach(lxb_dom_element_t *element);

LXB_API void
lxb_dom_document_stylesheet_destroy_all(lxb_dom_document_t *document,
                                        bool destroy_memory);

LXB_API lxb_status_t
lxb_dom_document_style_attach(lxb_dom_document_t *document,
                              lxb_css_rule_style_t *style);

LXB_API lxb_status_t
lxb_dom_document_style_remove(lxb_dom_document_t *document,
                              lxb_css_rule_style_t *style);

LXB_API lxb_status_t
lxb_dom_document_style_attach_by_element(lxb_dom_document_t *document,
                                         lxb_dom_element_t *element,
                                         lxb_css_rule_style_t *style);

LXB_API lxb_status_t
lxb_dom_document_apply_stylesheets(lxb_dom_document_t *document);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_DOM_DOCUMENT_H */
