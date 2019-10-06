/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_DOM_ATTR_H
#define LEXBOR_DOM_ATTR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/dom/interface.h"
#include "lexbor/dom/qualified_name.h"
#include "lexbor/dom/interfaces/node.h"


/* More memory to God of memory! */
/* TODO: Need hash value for name */
struct lxb_dom_attr {
    lxb_dom_node_t    node;

    lexbor_str_t      name;
    lexbor_str_t      local_name;
    lexbor_str_t      *value;

    lxb_dom_element_t *owner;

    lxb_dom_attr_t    *next;
    lxb_dom_attr_t    *prev;

    size_t            prefix_len;
};


LXB_API lxb_dom_attr_t *
lxb_dom_attr_interface_create(lxb_dom_document_t *document);

LXB_API lxb_dom_attr_t *
lxb_dom_attr_interface_destroy(lxb_dom_attr_t *attr);

LXB_API lxb_status_t
lxb_dom_attr_set_name(lxb_dom_attr_t *attr,
                      const lxb_char_t *local_name, size_t local_name_len,
                      const lxb_char_t *prefix, size_t prefix_len,
                      bool lowercase);

LXB_API lxb_status_t
lxb_dom_attr_set_name_wo_copy(lxb_dom_attr_t *attr,
                              lxb_char_t *local_name, size_t local_name_len,
                              const lxb_char_t *prefix, size_t prefix_len);

LXB_API lxb_status_t
lxb_dom_attr_set_value(lxb_dom_attr_t *attr,
                       const lxb_char_t *value, size_t value_len);

LXB_API lxb_status_t
lxb_dom_attr_set_value_wo_copy(lxb_dom_attr_t *attr,
                               lxb_char_t *value, size_t value_len);

LXB_API lxb_status_t
lxb_dom_attr_set_existing_value(lxb_dom_attr_t *attr,
                                const lxb_char_t *value, size_t value_len);

LXB_API lxb_status_t
lxb_dom_attr_clone_name_value(lxb_dom_attr_t *attr_from,
                              lxb_dom_attr_t *attr_to);

LXB_API bool
lxb_dom_attr_compare(lxb_dom_attr_t *first, lxb_dom_attr_t *second);


/*
 * Inline functions
 */
lxb_inline const lxb_char_t *
lxb_dom_attr_qualified_name(lxb_dom_attr_t *attr, size_t *len)
{
    if (len != NULL) {
        *len = attr->name.length;
    }

    return attr->name.data;
}

lxb_inline const lxb_char_t *
lxb_dom_attr_local_name(lxb_dom_attr_t *attr, size_t *len)
{
    if (len != NULL) {
        *len = attr->local_name.length;
    }

    return attr->local_name.data;
}

lxb_inline const lxb_char_t *
lxb_dom_attr_value(lxb_dom_attr_t *attr, size_t *len)
{
    if (attr->value == NULL) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len != NULL) {
        *len = attr->value->length;
    }

    return attr->value->data;
}

/*
 * No inline functions for ABI.
 */
const lxb_char_t *
lxb_dom_attr_qualified_name_noi(lxb_dom_attr_t *attr, size_t *len);

const lxb_char_t *
lxb_dom_attr_local_name_noi(lxb_dom_attr_t *attr, size_t *len);

const lxb_char_t *
lxb_dom_attr_value_noi(lxb_dom_attr_t *attr, size_t *len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_ATTR_H */
