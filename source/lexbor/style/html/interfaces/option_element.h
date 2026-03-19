/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_HTML_OPTION_ELEMENT_H
#define LEXBOR_STYLE_HTML_OPTION_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/style/html/interfaces/element.h"


LXB_API lxb_status_t
lxb_style_html_option_attr_change(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_style_html_option_attr_append(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_style_html_option_attr_remove(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_style_html_option_attr_replace(lxb_dom_element_t *element,
                                   lxb_dom_attr_id_t name,
                                   const lxb_char_t *old_value, size_t old_len,
                                   const lxb_char_t *value, size_t value_len,
                                   lxb_ns_id_t ns);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_HTML_OPTION_ELEMENT_H */
