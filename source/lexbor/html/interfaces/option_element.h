/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_OPTION_ELEMENT_H
#define LEXBOR_HTML_OPTION_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/interface.h"
#include "lexbor/html/interfaces/element.h"


struct lxb_html_option_element {
    lxb_html_element_t element;
    bool               selectedness;
};


LXB_API lxb_html_option_element_t *
lxb_html_option_element_interface_create(lxb_html_document_t *document);

LXB_API lxb_html_option_element_t *
lxb_html_option_element_interface_destroy(lxb_html_option_element_t *option_element);


LXB_API lxb_dom_exception_code_t
lxb_html_option_maybe_clone_to_selectedcontent(lxb_html_option_element_t *option);

LXB_API bool
lxb_html_option_element_selectedness(lxb_html_option_element_t *option);

LXB_API bool
lxb_html_option_is_disabled(lxb_html_option_element_t *option);

LXB_API lxb_dom_exception_code_t
lxb_html_option_update_nearest_ancestor_select(lxb_html_option_element_t *option);

LXB_API lxb_status_t
lxb_html_option_element_pop_open_elements(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_html_option_element_insert_steps(lxb_dom_node_t *inserted_node);

LXB_API lxb_status_t
lxb_html_option_element_remove_steps(lxb_dom_node_t *removed_node,
                                     lxb_dom_node_t *old_parent);

LXB_API lxb_status_t
lxb_html_option_attr_steps_change(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_html_option_attr_steps_remove(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_OPTION_ELEMENT_H */
