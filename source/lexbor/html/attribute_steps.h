/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_ATTRIBUTE_STEPS_H
#define LEXBOR_HTML_ATTRIBUTE_STEPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/dom/interfaces/attr.h"


LXB_API lxb_status_t
lxb_html_attribute_steps_change(lxb_dom_element_t *element,
                                lxb_dom_attr_id_t name,
                                const lxb_char_t *old_value, size_t old_len,
                                const lxb_char_t *value, size_t value_len,
                                lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_html_attribute_steps_append(lxb_dom_element_t *element,
                                lxb_dom_attr_id_t name,
                                const lxb_char_t *old_value, size_t old_len,
                                const lxb_char_t *value, size_t value_len,
                                lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_html_attribute_steps_remove(lxb_dom_element_t *element,
                                lxb_dom_attr_id_t name,
                                const lxb_char_t *old_value, size_t old_len,
                                const lxb_char_t *value, size_t value_len,
                                lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_html_attribute_steps_replace(lxb_dom_element_t *element,
                                 lxb_dom_attr_id_t name,
                                 const lxb_char_t *old_value, size_t old_len,
                                 const lxb_char_t *value, size_t value_len,
                                 lxb_ns_id_t ns);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_ATTRIBUTE_STEPS_H */
