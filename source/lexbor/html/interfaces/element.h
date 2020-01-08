/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_ELEMENT_H
#define LEXBOR_HTML_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/interface.h"
#include "lexbor/dom/interfaces/element.h"


struct lxb_html_element {
    lxb_dom_element_t element;
};


LXB_API lxb_html_element_t *
lxb_html_element_interface_create(lxb_html_document_t *document);

LXB_API lxb_html_element_t *
lxb_html_element_interface_destroy(lxb_html_element_t *element);


LXB_API lxb_html_element_t *
lxb_html_element_inner_html_set(lxb_html_element_t *element,
                                const lxb_char_t *html, size_t size);

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
