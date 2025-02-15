/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_HTML_ELEMENT_H
#define LEXBOR_STYLE_HTML_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/style/dom/interfaces/element.h"


/*
 * Inline functions
 */
lxb_inline const lxb_css_rule_declaration_t *
lxb_html_element_style_by_name(const lxb_html_element_t *element,
                               const lxb_char_t *name, size_t size)
{
    return lxb_dom_element_style_by_name(lxb_dom_interface_element(element),
                                         name, size);
}

lxb_inline const lxb_css_rule_declaration_t *
lxb_html_element_style_by_id(const lxb_html_element_t *element, uintptr_t id)
{
    return lxb_dom_element_style_by_id(lxb_dom_interface_element(element), id);
}

lxb_inline const lxb_style_node_t *
lxb_html_element_style_node_by_id(const lxb_html_element_t *element, uintptr_t id)
{
    return lxb_dom_element_style_node_by_id(lxb_dom_interface_element(element),
                                            id);
}

lxb_inline const lxb_style_node_t *
lxb_html_element_style_node_by_name(const lxb_html_element_t *element,
                                    const lxb_char_t *name, size_t size)
{
    return lxb_dom_element_style_node_by_name(lxb_dom_interface_element(element),
                                              name, size);
}

lxb_inline const void *
lxb_html_element_css_property_by_id(const lxb_html_element_t *element,
                                    uintptr_t id)
{
    return lxb_dom_element_css_property_by_id(lxb_dom_interface_element(element),
                                              id);
}

lxb_inline lxb_status_t
lxb_html_element_style_attach_exists(lxb_html_element_t *element)
{
    return lxb_dom_element_style_attach_exists(lxb_dom_interface_element(element));
}

lxb_inline lxb_status_t
lxb_html_element_style_attach(lxb_html_element_t *element,
                              lxb_css_rule_style_t *style)
{
    return lxb_dom_element_style_attach(lxb_dom_interface_element(element),
                                        style);
}

lxb_inline lxb_status_t
lxb_html_element_style_list_append(lxb_html_element_t *element,
                                   lxb_css_rule_declaration_list_t *list,
                                   lxb_css_selector_specificity_t spec)
{
    return lxb_dom_element_style_list_append(lxb_dom_interface_element(element),
                                             list, spec);
}

lxb_inline lxb_status_t
lxb_html_element_style_append(lxb_html_element_t *element,
                              lxb_css_rule_declaration_t *declr,
                              lxb_css_selector_specificity_t spec)
{
    return lxb_dom_element_style_append(lxb_dom_interface_element(element),
                                        declr, spec);
}

lxb_inline lxb_status_t
lxb_html_element_style_parse(lxb_html_element_t *element,
                             const lxb_char_t *style, size_t size)
{
    return lxb_dom_element_style_parse(lxb_dom_interface_element(element),
                                       style, size);
}

lxb_inline void
lxb_html_element_style_remove_by_name(lxb_html_element_t *element,
                                      const lxb_char_t *name, size_t size)
{
    lxb_dom_element_style_remove_by_name(lxb_dom_interface_element(element),
                                         name, size);
}

lxb_inline void
lxb_html_element_style_remove_by_id(lxb_html_element_t *element, uintptr_t id)
{
    lxb_dom_element_style_remove_by_id(lxb_dom_interface_element(element), id);
}

lxb_inline lxb_status_t
lxb_html_element_style_serialize(lxb_html_element_t *element,
                                 lxb_dom_element_style_opt_t opt,
                                 lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_dom_element_style_serialize(lxb_dom_interface_element(element),
                                           opt, cb, ctx);
}

lxb_inline lxb_status_t
lxb_html_element_style_serialize_str(lxb_html_element_t *element,
                                     lexbor_str_t *str,
                                     lxb_dom_element_style_opt_t opt)
{
    return lxb_dom_element_style_serialize_str(lxb_dom_interface_element(element),
                                               str, opt);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_HTML_ELEMENT_H */
