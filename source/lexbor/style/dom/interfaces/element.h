/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_DOM_ELEMENT_H
#define LEXBOR_STYLE_DOM_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/style/base.h"
#include "lexbor/html/interface.h"
#include "lexbor/dom/interfaces/element.h"
#include "lexbor/css/rule.h"


typedef enum {
    LXB_DOM_ELEMENT_STYLE_OPT_UNDEF = 0x00
}
lxb_dom_element_style_opt_t;


typedef lxb_status_t
(*lxb_dom_element_style_cb_f)(lxb_dom_element_t *element,
                              const lxb_css_rule_declaration_t *declr,
                              void *ctx, lxb_css_selector_specificity_t spec,
                              bool is_weak);


LXB_API const lxb_css_rule_declaration_t *
lxb_dom_element_style_by_name(const lxb_dom_element_t *element,
                              const lxb_char_t *name, size_t size);

LXB_API const lxb_css_rule_declaration_t *
lxb_dom_element_style_by_id(const lxb_dom_element_t *element, uintptr_t id);

LXB_API const lxb_style_node_t *
lxb_dom_element_style_node_by_id(const lxb_dom_element_t *element, uintptr_t id);

LXB_API const lxb_style_node_t *
lxb_dom_element_style_node_by_name(const lxb_dom_element_t *element,
                                   const lxb_char_t *name, size_t size);

LXB_API const void *
lxb_dom_element_css_property_by_id(const lxb_dom_element_t *element,
                                   uintptr_t id);

LXB_API lxb_status_t
lxb_dom_element_style_attach_exists(lxb_dom_element_t *element);

LXB_API lxb_status_t
lxb_dom_element_style_attach(lxb_dom_element_t *element,
                             lxb_css_rule_style_t *style);

LXB_API lxb_status_t
lxb_dom_element_style_list_append(lxb_dom_element_t *element,
                                  lxb_css_rule_declaration_list_t *list,
                                  lxb_css_selector_specificity_t spec);

LXB_API lxb_status_t
lxb_dom_element_style_append(lxb_dom_element_t *element,
                             lxb_css_rule_declaration_t *declr,
                             lxb_css_selector_specificity_t spec);

LXB_API lxb_status_t
lxb_dom_element_style_weak_append(lxb_dom_document_t *doc,
                                  lxb_style_node_t *node,
                                  lxb_css_rule_declaration_t *declr,
                                  lxb_css_selector_specificity_t spec);

LXB_API lxb_status_t
lxb_dom_element_style_walk(lxb_dom_element_t *element,
                           lxb_dom_element_style_cb_f cb,
                           void *ctx, bool with_weak);

LXB_API lxb_status_t
lxb_dom_element_style_parse(lxb_dom_element_t *element,
                            const lxb_char_t *style, size_t size);

LXB_API void
lxb_dom_element_style_remove_by_name(lxb_dom_element_t *element,
                                     const lxb_char_t *name, size_t size);

LXB_API void
lxb_dom_element_style_remove_by_id(lxb_dom_element_t *element, uintptr_t id);

LXB_API lxb_style_node_t *
lxb_dom_element_style_remove_all_not(lxb_dom_document_t *doc,
                                     lexbor_avl_node_t **root,
                                     lxb_style_node_t *style, bool bs);

LXB_API lxb_style_node_t *
lxb_dom_element_style_remove_all(lxb_dom_document_t *doc,
                                 lexbor_avl_node_t **root,
                                 lxb_style_node_t *style);

LXB_API lxb_style_node_t *
lxb_dom_element_style_remove_by_list(lxb_dom_document_t *doc,
                                     lexbor_avl_node_t **root,
                                     lxb_style_node_t *style,
                                     lxb_css_rule_declaration_list_t *list);

LXB_API lxb_status_t
lxb_dom_element_style_serialize(lxb_dom_element_t *element,
                                lxb_dom_element_style_opt_t opt,
                                lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_status_t
lxb_dom_element_style_serialize_str(lxb_dom_element_t *element,
                                    lexbor_str_t *str,
                                    lxb_dom_element_style_opt_t opt);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_DOM_ELEMENT_H */

