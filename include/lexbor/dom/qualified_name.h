/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_DOM_QUALIFIED_NAME_H
#define LEXBOR_DOM_QUALIFIED_NAME_H

#ifdef __cplusplus
//extern "C" {
#endif

#include <lexbor/core/base.h>
#include <lexbor/core/str.h>

#include <lexbor/dom/interface.h>


typedef struct {
    lexbor_str_t str;
    lxb_char_t   *upper;         /* This is a upper case copy of str.name */

    unsigned int prefix_len;
    unsigned int local_name_len;
}
lxb_dom_qualified_name_t;


LXB_API lxb_dom_qualified_name_t *
lxb_dom_qualified_name_make(lxb_dom_document_t *document,
                            const lxb_char_t *prefix, unsigned int prefix_len,
                            const lxb_char_t *lname, unsigned int lname_len);

LXB_API lxb_status_t
lxb_dom_qualified_name_change(lxb_dom_document_t *document,
                              lxb_dom_qualified_name_t *qname,
                              const lxb_char_t *prefix, unsigned int prefix_len,
                              const lxb_char_t *lname, unsigned int lname_len);

LXB_API const lxb_char_t *
lxb_dom_qualified_name_upper(lxb_dom_document_t *document,
                             lxb_dom_qualified_name_t *qname, size_t *len);


/*
 * Inline functions
 */
lxb_inline const lxb_char_t *
lxb_dom_qualified_name(lxb_dom_qualified_name_t *qname, size_t *len)
{
    if (len != NULL) {
        *len = qname->str.length;
    }

    return qname->str.data;
}

lxb_inline const lxb_char_t *
lxb_dom_qualified_name_prefix(lxb_dom_qualified_name_t *qname, size_t *len)
{
    if (len != NULL) {
        *len = qname->prefix_len;
    }

    return qname->str.data;
}

lxb_inline const lxb_char_t *
lxb_dom_qualified_name_local_name(lxb_dom_qualified_name_t *qname, size_t *len)
{
    if (qname->local_name_len == 0) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len != NULL) {
        *len = qname->local_name_len;
    }

    if (qname->prefix_len != 0) {
        /*
         * In str.data: "prefix:localname".
         * Need skip "prefix:".
         */
        return &qname->str.data[ (qname->prefix_len + 1) ];
    }

    /* In str.data: "localname" */
    return qname->str.data;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_QUALIFIED_NAME_H */
