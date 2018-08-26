/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_HTML_DOCUMENT_H
#define LEXBOR_HTML_DOCUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/mraw.h>

#include <lexbor/tag/tag.h>
#include <lexbor/html/interface.h>
#include <lexbor/dom/interfaces/document.h>


typedef enum {
    LXB_HTML_DOCUMENT_READY_STATE_LOADING     = 0x01,
    LXB_HTML_DOCUMENT_READY_STATE_INTERACTIVE = 0x02,
    LXB_HTML_DOCUMENT_READY_STATE_COMPLETE    = 0x03,
}
lxb_html_document_ready_state_t;

typedef struct {
    lexbor_mraw_t  *mraw;
    lexbor_mraw_t  *text;
    lxb_tag_heap_t *tag_heap_ref;
}
lxb_html_document_mem_t;

struct lxb_html_document {
    lxb_dom_document_t              dom_document;

    void                            *iframe_srcdoc;

    lxb_html_head_element_t         *head;
    lxb_html_body_element_t         *body;

    lxb_html_document_mem_t         *mem;

    lxb_html_document_ready_state_t ready_state;
};

LXB_API lxb_html_document_t *
lxb_html_document_create(lxb_html_document_t *document);

LXB_API lxb_status_t
lxb_html_document_init(lxb_html_document_t *document, lxb_tag_heap_t *tag_heap);

LXB_API lxb_html_document_t *
lxb_html_document_destroy(lxb_html_document_t *document);


/*
 * Inline functions
 */
lxb_inline lxb_html_head_element_t *
lxb_html_document_head_element(lxb_html_document_t *document)
{
    return document->head;
}

lxb_inline lxb_html_body_element_t *
lxb_html_document_body_element(lxb_html_document_t *document)
{
    return document->body;
}

lxb_inline lxb_dom_document_t *
lxb_html_document_original_ref(lxb_html_document_t *document)
{
    if (lxb_dom_interface_node(document)->owner_document
        != &document->dom_document)
    {
        return lxb_dom_interface_node(document)->owner_document;
    }

    return lxb_dom_interface_document(document);
}

lxb_inline bool
lxb_html_document_is_original(lxb_html_document_t *document)
{
    return lxb_dom_interface_node(document)->owner_document
        == &document->dom_document;
}

lxb_inline lexbor_mraw_t*
lxb_html_document_mraw(lxb_html_document_t *document)
{
    return document->mem->mraw;
}

lxb_inline lexbor_mraw_t*
lxb_html_document_mraw_text(lxb_html_document_t *document)
{
    return document->mem->text;
}

lxb_inline lxb_tag_heap_t *
lxb_html_document_tag_heap(lxb_html_document_t *document)
{
    return document->mem->tag_heap_ref;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_DOCUMENT_H */
