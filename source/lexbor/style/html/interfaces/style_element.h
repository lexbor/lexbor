/*
 * Copyright (C) 2025-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_HTML_STYLE_ELEMENT_H
#define LEXBOR_STYLE_HTML_STYLE_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/style/html/interfaces/document.h"


LXB_API lxb_status_t
lxb_html_style_element_parse(lxb_html_style_element_t *element);

LXB_API lxb_status_t
lxb_html_style_element_remove(lxb_html_style_element_t *element);

LXB_API lxb_status_t
lxb_html_style_element_pop_open_elements(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_html_style_element_insert_steps(lxb_dom_node_t *inserted_node);

LXB_API lxb_status_t
lxb_html_style_element_remove_steps(lxb_dom_node_t *removed_node,
                                    lxb_dom_node_t *old_parent);

LXB_API lxb_status_t
lxb_html_style_element_destroy_steps(lxb_dom_node_t *node);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_HTML_STYLE_ELEMENT_H */
