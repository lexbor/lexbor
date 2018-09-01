/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/qualified_name.h"
#include "lexbor/dom/interfaces/document.h"

#define LEXBOR_STR_RES_MAP_UPPERCASE
#include "lexbor/core/str_res.h"


lxb_inline void
lxb_dom_qualified_name_set(lxb_dom_qualified_name_t *qname, size_t size,
                           const lxb_char_t *prefix, unsigned int prefix_len,
                           const lxb_char_t *lname, unsigned int lname_len);


lxb_dom_qualified_name_t *
lxb_dom_qualified_name_make(lxb_dom_document_t *document,
                            const lxb_char_t *prefix, unsigned int prefix_len,
                            const lxb_char_t *lname, unsigned int lname_len)
{
    size_t size;
    lxb_dom_qualified_name_t *qname;

    qname = lexbor_mraw_calloc(document->mraw,
                               sizeof(lxb_dom_qualified_name_t));
    if (qname == NULL) {
        return NULL;
    }

    size = (prefix_len + lname_len + 1);

    lexbor_str_init(&qname->str, document->text, size);
    if (qname->str.data == NULL) {
        return lexbor_mraw_free(document->text, qname);
    }

    lxb_dom_qualified_name_set(qname, size, prefix, prefix_len,
                               lname, lname_len);

    return qname;
}

lxb_status_t
lxb_dom_qualified_name_change(lxb_dom_document_t *document,
                              lxb_dom_qualified_name_t *qname,
                              const lxb_char_t *prefix, unsigned int prefix_len,
                              const lxb_char_t *lname, unsigned int lname_len)
{
    size_t size = (prefix_len + lname_len + 1);

    if (size > qname->str.length) {
        lxb_char_t *tmp = lexbor_str_realloc(&qname->str, document->text, size);

        if (tmp == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    lxb_dom_qualified_name_set(qname, size, prefix, prefix_len,
                               lname, lname_len);

    return LXB_STATUS_OK;
}

lxb_inline void
lxb_dom_qualified_name_set(lxb_dom_qualified_name_t *qname, size_t size,
                           const lxb_char_t *prefix, unsigned int prefix_len,
                           const lxb_char_t *lname, unsigned int lname_len)
{
    if (prefix_len != 0) {
        memcpy(qname->str.data, prefix, sizeof(lxb_char_t) * prefix_len);

        /* U+003A COLON (:) */
        qname->str.data[prefix_len] = 0x3A;

        memcpy(&qname->str.data[ (prefix_len + 1) ],
               lname, sizeof(lxb_char_t) * lname_len);

        qname->str.length = size;
        qname->str.data[size] = 0x00;
    }
    else if (lname_len != 0) {
        memcpy(qname->str.data, lname, sizeof(lxb_char_t) * lname_len);

        qname->str.length = lname_len;
        qname->str.data[lname_len] = 0x00;
    }

    qname->prefix_len = prefix_len;
    qname->local_name_len = lname_len;
}

const lxb_char_t *
lxb_dom_qualified_name_upper(lxb_dom_document_t *document,
                             lxb_dom_qualified_name_t *qname, size_t *len)
{
    if (len != NULL) {
        *len = qname->str.length;
    }

    if (qname->upper != NULL) {
        return qname->upper;
    }

    qname->upper = lexbor_mraw_alloc(document->text, (qname->str.length + 1));
    if (qname->upper == NULL) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    for (size_t i = 0; i < qname->str.length; i++) {
        qname->upper[i] = lexbor_str_res_map_uppercase[ qname->str.data[i] ];
    }

    qname->upper[ qname->str.length ] = 0x00;

    return qname->upper;
}
