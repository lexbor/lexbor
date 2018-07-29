/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/interfaces/document.h"


lxb_html_document_t *
lxb_html_document_create(lxb_html_document_t *document)
{
    if (document != NULL) {
        lxb_html_document_t *doc;

        doc = lexbor_mraw_calloc(document->mem->mraw,
                                 sizeof(lxb_html_document_t));
        if (doc == NULL) {
            return NULL;
        }

        doc->dom_document.type = LXB_DOM_DOCUMENT_DTYPE_HTML;

        lxb_dom_node_t *node = lxb_dom_interface_node(doc);

        node->owner_document = lxb_dom_interface_document(document);
        node->type = LXB_DOM_NODE_TYPE_DOCUMENT;

        node->tag_id = LXB_HTML_TAG__DOCUMENT;
        node->ns = LXB_HTML_NS_HTML;

        return doc;
    }

    lxb_dom_node_t *node;
    lxb_html_document_t *doc;
    lexbor_mraw_t *mraw, *text;

    /* For nodes */
    mraw = lexbor_mraw_create();
    lxb_status_t status = lexbor_mraw_init(mraw, (4096 * 8));

    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    /* For text */
    text = lexbor_mraw_create();
    status = lexbor_mraw_init(text, (4096 * 12));

    if (status != LXB_STATUS_OK) {
        return (void *) lexbor_mraw_destroy(mraw, true);
    }

    doc = lexbor_mraw_calloc(mraw, sizeof(lxb_html_document_t));
    if (doc == NULL) {
        goto failure;
    }

    doc->mem = lexbor_mraw_calloc(mraw, sizeof(lxb_html_document_mem_t));
    if (doc->mem == NULL) {
        goto failure;
    }

    doc->mem->mraw = mraw;
    doc->mem->text = text;

    doc->dom_document.type = LXB_DOM_DOCUMENT_DTYPE_HTML;

    node = lxb_dom_interface_node(doc);

    node->owner_document = lxb_dom_interface_document(doc);
    node->type = LXB_DOM_NODE_TYPE_DOCUMENT;

    node->tag_id = LXB_HTML_TAG__DOCUMENT;
    node->ns = LXB_HTML_NS_HTML;

    node->owner_document->mraw = mraw;
    node->owner_document->text = text;

    return doc;

failure:

    lexbor_mraw_destroy(mraw, true);
    lexbor_mraw_destroy(text, true);

    return NULL;
}

lxb_status_t
lxb_html_document_init(lxb_html_document_t *document,
                       lxb_html_tag_heap_t *tag_heap)
{
    lxb_status_t status;

    if (document == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (tag_heap != NULL) {
        document->mem->tag_heap_ref = lxb_html_tag_heap_ref(tag_heap);
        document->dom_document.tags = tag_heap;

        return LXB_STATUS_OK;
    }

    tag_heap = lxb_html_tag_heap_create();
    status = lxb_html_tag_heap_init(tag_heap, 128);

    if (status != LXB_STATUS_OK) {
        lexbor_mraw_destroy(document->mem->mraw, true);

        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    document->mem->tag_heap_ref = tag_heap;
    document->dom_document.tags = tag_heap;

    return LXB_STATUS_OK;
}

lxb_html_document_t *
lxb_html_document_destroy(lxb_html_document_t *document)
{
    if (document == NULL) {
        return NULL;
    }

    if (document->mem == NULL) {
        lxb_dom_document_t *owner;

        /*
         * We can get lxb_html_document_t from owner_document:
         * lxb_html_interface_document(owner_document)
         */
        owner = lxb_dom_interface_node(document)->owner_document;

        return lexbor_mraw_free(owner->mraw, document);
    }

    document->dom_document.tags = NULL;

    lxb_html_tag_heap_unref(document->mem->tag_heap_ref, true);
    lexbor_mraw_destroy(document->mem->mraw, true);

    return NULL;
}
