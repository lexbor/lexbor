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
    lxb_dom_node_t              node;

    lxb_dom_document_cmode_t    compat_mode;
    lxb_dom_document_dtype_t    type;

    lxb_dom_document_type_t     *doctype;
    lxb_dom_element_t           *element;

    lxb_dom_interface_create_f  create_interface;
    lxb_dom_interface_destroy_f destroy_interface;

    lexbor_mraw_t               *mraw;
    lexbor_mraw_t               *text;
    void                        *tags;
    void                        *ns;

    bool                        scripting;
};


LXB_API lxb_dom_document_t *
lxb_dom_document_interface_create(lxb_dom_document_t *document);

LXB_API lxb_dom_document_t *
lxb_dom_document_interface_destroy(lxb_dom_document_t *document);

LXB_API void
lxb_dom_document_attach_doctype(lxb_dom_document_t *document,
                                lxb_dom_document_type_t *doctype);

LXB_API void
lxb_dom_document_attach_element(lxb_dom_document_t *document,
                                lxb_dom_element_t *element);

lxb_dom_element_t *
lxb_dom_document_create_element(lxb_dom_document_t *document,
                                const lxb_char_t *local_name, size_t lname_len,
                                void *reserved_for_opt);

lxb_dom_text_t *
lxb_dom_document_create_text_node(lxb_dom_document_t *document,
                                  const lxb_char_t *data, size_t data_len);


/*
 * Inline functions
 */
lxb_inline lxb_dom_interface_t *
lxb_dom_document_create_interface(lxb_dom_document_t *document,
                                  lxb_tag_id_t tag_id, lxb_ns_id_t ns)
{
    return document->create_interface(document, tag_id, ns);
}

lxb_inline lxb_dom_interface_t *
lxb_dom_document_destroy_interface(lxb_dom_interface_t *interface)
{
    return lxb_dom_interface_node(interface)->owner_document->destroy_interface(interface);
}

lxb_inline void *
lxb_dom_document_create_struct(lxb_dom_document_t *document, size_t struct_size)
{
    return lexbor_mraw_calloc(document->mraw, struct_size);
}

lxb_inline void *
lxb_dom_document_destroy_struct(lxb_dom_document_t *document, void *structure)
{
    return lexbor_mraw_free(document->mraw, structure);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_DOCUMENT_H */
