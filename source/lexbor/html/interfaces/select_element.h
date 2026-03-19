/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_SELECT_ELEMENT_H
#define LEXBOR_HTML_SELECT_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/interface.h"
#include "lexbor/html/interfaces/element.h"


typedef lxb_status_t
(*lxb_html_select_options_cb_f)(lxb_html_select_element_t *el,
                                lxb_html_option_element_t *option, void *ctx);


struct lxb_html_select_element {
    lxb_html_element_t element;
};


LXB_API lxb_html_select_element_t *
lxb_html_select_element_interface_create(lxb_html_document_t *document);

LXB_API lxb_html_select_element_t *
lxb_html_select_element_interface_destroy(lxb_html_select_element_t *select_element);

LXB_API lxb_html_selectedcontent_element_t *
lxb_html_select_get_enabled_selectedcontent(lxb_html_select_element_t *el);

LXB_API lxb_status_t
lxb_html_select_list_of_options(lxb_html_select_element_t *el,
                                lxb_html_select_options_cb_f walker_cb, void *ctx);

LXB_API lxb_dom_exception_code_t
lxb_html_select_selectedness_setting_algorithm(lxb_html_select_element_t *el);

LXB_API lxb_status_t
lxb_html_select_element_insert_steps(lxb_dom_node_t *inserted_node);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_SELECT_ELEMENT_H */
