/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/core/str.h"

#include "lexbor/html/interfaces/document.h"
#include "lexbor/html/html.h"
#include "lexbor/tag/tag.h"

#include "lexbor/dom/interfaces/text.h"

#define LXB_HTML_TAG_RES_DATA
#define LXB_HTML_TAG_RES_SHS_DATA
#include "lexbor/html/tag_res.h"


lxb_html_document_t *
lxb_html_document_create(lxb_html_document_t *document)
{
    lxb_dom_node_t *node;
    lxb_html_document_t *doc;
    lexbor_mraw_t *mraw, *text;

    if (document != NULL) {
        doc = lexbor_mraw_calloc(document->mem->mraw,
                                 sizeof(lxb_html_document_t));
        if (doc == NULL) {
            return NULL;
        }

        doc->mem = document->mem;

        doc->dom_document.type = LXB_DOM_DOCUMENT_DTYPE_HTML;
        doc->dom_document.mraw = document->mem->mraw;
        doc->dom_document.text = document->mem->text;
        doc->dom_document.create_interface = (lxb_dom_interface_create_f) lxb_html_interface_create;
        doc->dom_document.destroy_interface = lxb_html_interface_destroy;

        node = lxb_dom_interface_node(doc);
        node->owner_document = lxb_dom_interface_document(document);

        node->type = LXB_DOM_NODE_TYPE_DOCUMENT;
        node->tag_id = LXB_TAG__DOCUMENT;
        node->ns = LXB_NS_HTML;

        return doc;
    }

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
    doc->dom_document.mraw = mraw;
    doc->dom_document.text = text;
    doc->dom_document.create_interface = (lxb_dom_interface_create_f) lxb_html_interface_create;
    doc->dom_document.destroy_interface = lxb_html_interface_destroy;

    node = lxb_dom_interface_node(doc);
    node->owner_document = lxb_dom_interface_document(doc);

    node->type = LXB_DOM_NODE_TYPE_DOCUMENT;
    node->tag_id = LXB_TAG__DOCUMENT;
    node->ns = LXB_NS_HTML;

    return doc;

failure:

    lexbor_mraw_destroy(mraw, true);
    lexbor_mraw_destroy(text, true);

    return NULL;
}

lxb_status_t
lxb_html_document_init(lxb_html_document_t *document,
                       lxb_tag_heap_t *tag_heap, lxb_ns_heap_t *ns_heap)
{
    lxb_status_t status;

    if (document == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (tag_heap != NULL) {
        document->mem->tag_heap_ref = lxb_tag_heap_ref(tag_heap);
        document->dom_document.tags = tag_heap;
    }
    else {
        tag_heap = lxb_tag_heap_create();
        status = lxb_tag_heap_init(tag_heap, 128);

        if (status != LXB_STATUS_OK) {
            lxb_tag_heap_destroy(tag_heap);

            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        document->mem->tag_heap_ref = tag_heap;
        document->dom_document.tags = tag_heap;
    }

    if (ns_heap != NULL) {
        document->mem->ns_heap_ref = lxb_ns_heap_ref(ns_heap);
        document->dom_document.ns = ns_heap;

        return LXB_STATUS_OK;
    }

    ns_heap = lxb_ns_heap_create();
    status = lxb_ns_heap_init(ns_heap, 128);

    if (status != LXB_STATUS_OK) {
        lxb_ns_heap_destroy(ns_heap);

        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    document->mem->ns_heap_ref = ns_heap;
    document->dom_document.ns = ns_heap;

    return LXB_STATUS_OK;
}

lxb_html_document_t *
lxb_html_document_destroy(lxb_html_document_t *document)
{
    if (document == NULL) {
        return NULL;
    }

    if (lxb_dom_interface_node(document)->owner_document
        != &document->dom_document)
    {
        lxb_dom_document_t *owner;

        /*
         * We can get lxb_html_document_t from owner_document:
         * lxb_html_interface_document(owner_document)
         */
        owner = lxb_dom_interface_node(document)->owner_document;

        return lexbor_mraw_free(owner->mraw, document);
    }

    document->dom_document.tags = NULL;

    lxb_tag_heap_unref(document->mem->tag_heap_ref);
    lxb_ns_heap_unref(document->mem->ns_heap_ref);
    lexbor_mraw_destroy(document->mem->mraw, true);

    return NULL;
}
