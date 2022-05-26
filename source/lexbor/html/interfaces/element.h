/*
 * Copyright (C) 2018-2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_ELEMENT_H
#define LEXBOR_HTML_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/avl.h"

#include "lexbor/html/interface.h"
#include "lexbor/dom/interfaces/element.h"
#include "lexbor/css/rule.h"


struct lxb_html_element {
    lxb_dom_element_t element;
    lexbor_avl_node_t *style;
};


LXB_API lxb_html_element_t *
lxb_html_element_interface_create(lxb_html_document_t *document);

LXB_API lxb_html_element_t *
lxb_html_element_interface_destroy(lxb_html_element_t *element);


LXB_API lxb_html_element_t *
lxb_html_element_inner_html_set(lxb_html_element_t *element,
                                const lxb_char_t *html, size_t size);

LXB_API const lxb_css_rule_declaration_t *
lxb_html_element_style_by_name(lxb_html_element_t *element,
                               const lxb_char_t *name, size_t size);

LXB_API const lxb_css_rule_declaration_t *
lxb_html_element_style_by_id(lxb_html_element_t *element, uintptr_t id);

LXB_API lxb_status_t
lxb_html_element_style_parse(lxb_html_element_t *element,
                             const lxb_char_t *style, size_t size);

LXB_API lxb_status_t
lxb_html_element_style_append(lxb_html_element_t *element,
                              lxb_css_rule_declaration_t *declr,
                              lxb_css_selector_specificity_t spec);

LXB_API lxb_status_t
lxb_html_element_style_list_append(lxb_html_element_t *element,
                                   lxb_css_rule_declaration_list_t *list,
                                   lxb_css_selector_specificity_t spec);

/*
 * Inline functions
 */
lxb_inline lxb_tag_id_t
lxb_html_element_tag_id(lxb_html_element_t *element)
{
    return lxb_dom_interface_node(element)->local_name;
}

lxb_inline lxb_ns_id_t
lxb_html_element_ns_id(lxb_html_element_t *element)
{
    return lxb_dom_interface_node(element)->ns;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_ELEMENT_H */
