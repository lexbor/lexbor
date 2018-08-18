/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_DOM_DOCUMENT_H
#define LEXBOR_DOM_DOCUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/mraw.h>

#include <lexbor/dom/interface.h>
#include <lexbor/dom/interfaces/node.h>


typedef enum {
    LXB_DOM_DOCUMENT_CMODE_NO_QUIRKS       = 0x00,
    LXB_DOM_DOCUMENT_CMODE_QUIRKS          = 0x01,
    LXB_DOM_DOCUMENT_CMODE_LIMITED_QUIRKS  = 0x02
}
lxb_dom_document_cmode_t;

typedef enum {
    LXB_DOM_DOCUMENT_DTYPE_UNDEF = 0x00,
    LXB_DOM_DOCUMENT_DTYPE_HTML  = 0x01,
    LXB_DOM_DOCUMENT_DTYPE_XML   = 0x02
}
lxb_dom_document_dtype_t;

struct lxb_dom_document {
    lxb_dom_node_t           node;

    lxb_dom_document_cmode_t compat_mode;
    lxb_dom_document_dtype_t type;

    lxb_dom_document_type_t  *doctype;
    lxb_dom_element_t        *element;

    lexbor_mraw_t            *mraw;
    lexbor_mraw_t            *text;
    void                     *tags;

    bool                     scripting;
};


LXB_API lxb_dom_document_t *
lxb_dom_document_create(lxb_dom_document_t *document);

LXB_API lxb_dom_document_t *
lxb_dom_document_destroy(lxb_dom_document_t *document);

LXB_API void
lxb_dom_document_attach_doctype(lxb_dom_document_t *document,
                                lxb_dom_document_type_t *doctype);

LXB_API void
lxb_dom_document_attach_element(lxb_dom_document_t *document,
                                lxb_dom_element_t *element);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_DOCUMENT_H */
