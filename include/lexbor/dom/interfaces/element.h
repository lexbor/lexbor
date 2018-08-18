/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_DOM_ELEMENT_H
#define LEXBOR_DOM_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/str.h>

#include <lexbor/dom/interfaces/document.h>
#include <lexbor/dom/interfaces/node.h>


struct lxb_dom_element {
    lxb_dom_node_t         node;

    lexbor_str_t           *is_value;

    lxb_dom_element_attr_t *first_attr;
    lxb_dom_element_attr_t *last_attr;
};

struct lxb_dom_element_attr {
    unsigned int           ns;

    lexbor_str_t           local_name;
    lexbor_str_t           name;
    lexbor_str_t           value;

    lxb_dom_element_attr_t *next;
    lxb_dom_element_attr_t *prev;

    lxb_dom_element_t      *owner;
};


LXB_API lxb_dom_element_t *
lxb_dom_element_create(lxb_dom_document_t *document);

LXB_API lxb_dom_element_t *
lxb_dom_element_destroy(lxb_dom_element_t *element);

LXB_API bool
lxb_dom_element_compare(lxb_dom_element_t *first, lxb_dom_element_t *second);

LXB_API bool
lxb_dom_element_attr_compare(lxb_dom_element_attr_t *first,
                             lxb_dom_element_attr_t *second);

LXB_API lxb_dom_element_attr_t *
lxb_dom_element_attr_is_exist(lxb_dom_element_t *element,
                              const lxb_char_t *name, size_t len);

LXB_API lxb_dom_element_attr_t *
lxb_dom_element_attr_create(lxb_dom_element_t *element);

LXB_API lxb_dom_element_t *
lxb_dom_element_attr_destroy(lxb_dom_element_attr_t *attr);

LXB_API void
lxb_dom_element_attr_append(lxb_dom_element_t *element,
                            lxb_dom_element_attr_t *attr);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_ELEMENT_H */
