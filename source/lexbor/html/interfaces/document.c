/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/core/str.h"

#include "lexbor/html/interfaces/document.h"
#include "lexbor/html/interfaces/title_element.h"
#include "lexbor/html/html.h"
#include "lexbor/html/parser.h"

#include "lexbor/tag/tag.h"

#include "lexbor/dom/interfaces/text.h"
#include "lexbor/dom/interfaces/element.h"

#define LXB_HTML_TAG_RES_DATA
#define LXB_HTML_TAG_RES_SHS_DATA
#include "lexbor/html/tag_res.h"


lxb_inline lxb_status_t
lxb_html_document_parse_prepare(lxb_html_document_t *document);

static lexbor_action_t
lxb_html_document_title_walker(lxb_dom_node_t *node, void *ctx);


lxb_html_document_t *
lxb_html_document_interface_create(lxb_html_document_t *document)
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
        return (void *) lexbor_mraw_destroy(mraw, true);
    }

    /* For text */
    text = lexbor_mraw_create();
    status = lexbor_mraw_init(text, (4096 * 12));

    if (status != LXB_STATUS_OK) {
        goto failure;
    }

    doc = lexbor_mraw_calloc(mraw, sizeof(lxb_html_document_t));
    if (doc == NULL) {
        goto failure;
    }

    doc->mem = lexbor_mraw_calloc(mraw, sizeof(lxb_html_document_mem_t));
    if (doc->mem == NULL) {
        lexbor_mraw_free(mraw, doc);

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
lxb_html_document_interface_init(lxb_html_document_t *document,
                                 lxb_tag_heap_t *tag_heap,
                                 lxb_ns_heap_t *ns_heap)
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
lxb_html_document_interface_destroy(lxb_html_document_t *document)
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

    lxb_tag_heap_unref(document->mem->tag_heap_ref);
    lxb_ns_heap_unref(document->mem->ns_heap_ref);
    lxb_html_parser_destroy(document->mem->parser, true);
    lexbor_mraw_destroy(document->mem->text, true);

    /*
     * Do not move it! Always should be called in the last turn!
     * First we create `mraw`, then we create `mem`
     * and save pointer `mraw` to `mem`.
     */
    lexbor_mraw_destroy(document->mem->mraw, true);

    return NULL;
}

lxb_html_document_t *
lxb_html_document_create(void)
{
    lxb_html_document_t *doc = lxb_html_document_interface_create(NULL);
    if (doc == NULL) {
        return NULL;
    }

    lxb_status_t status = lxb_html_document_interface_init(doc, NULL, NULL);
    if (status != LXB_STATUS_OK) {
        return lxb_html_document_destroy(doc);
    }

    return doc;
}

void
lxb_html_document_clean(lxb_html_document_t *document)
{
    lxb_tag_heap_clean(document->mem->tag_heap_ref);
    lxb_ns_heap_clean(document->mem->ns_heap_ref);

    lexbor_mraw_clean(document->mem->mraw);
    lexbor_mraw_clean(document->mem->text);
}

lxb_html_document_t *
lxb_html_document_destroy(lxb_html_document_t *document)
{
    return lxb_html_document_interface_destroy(document);
}

lxb_status_t
lxb_html_document_parse(lxb_html_document_t *document,
                        const lxb_char_t *html, size_t size)
{
    if (document->ready_state != LXB_HTML_DOCUMENT_READY_STATE_UNDEF
        && document->ready_state != LXB_HTML_DOCUMENT_READY_STATE_LOADING)
    {
        lxb_html_document_clean(document);
    }

    lxb_html_document_opt_t opt = document->opt;

    lxb_status_t status = lxb_html_document_parse_prepare(document);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    status = lxb_html_parse_chunk_prepare(document->mem->parser, document);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    status = lxb_html_parse_chunk_process(document->mem->parser, html, size);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    document->opt = opt;

    return lxb_html_parse_chunk_end(document->mem->parser);

failed:

    document->opt = opt;

    return status;
}

lxb_status_t
lxb_html_document_parse_chunk_begin(lxb_html_document_t *document)
{
    if (document->ready_state != LXB_HTML_DOCUMENT_READY_STATE_UNDEF
        && document->ready_state != LXB_HTML_DOCUMENT_READY_STATE_LOADING)
    {
        lxb_html_document_clean(document);
    }

    lxb_status_t status = lxb_html_document_parse_prepare(document);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_html_parse_chunk_prepare(document->mem->parser, document);
}

lxb_status_t
lxb_html_document_parse_chunk(lxb_html_document_t *document,
                              const lxb_char_t *html, size_t size)
{
    if (document->mem == NULL) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    return lxb_html_parse_chunk_process(document->mem->parser, html, size);
}

lxb_status_t
lxb_html_document_parse_chunk_end(lxb_html_document_t *document)
{
    if (document->mem == NULL) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    return lxb_html_parse_chunk_end(document->mem->parser);
}

lxb_dom_node_t *
lxb_html_document_parse_fragment(lxb_html_document_t *document,
                                 lxb_dom_element_t *element,
                                 const lxb_char_t *html, size_t size)
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
    lxb_html_document_opt_t opt = document->opt;

    status = lxb_html_document_parse_prepare(document);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    parser = document->mem->parser;

    status = lxb_html_parse_fragment_chunk_begin(parser, document,
                                                 element->node.tag_id,
                                                 element->node.ns);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    status = lxb_html_parse_fragment_chunk_process(parser, html, size);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    document->opt = opt;

    return lxb_html_parse_fragment_chunk_end(parser);

failed:

    document->opt = opt;

    return NULL;
}

lxb_status_t
lxb_html_document_parse_fragment_chunk_begin(lxb_html_document_t *document,
                                             lxb_dom_element_t *element)
{
    lxb_status_t status;
    lxb_html_parser_t *parser = document->mem->parser;;

    status = lxb_html_document_parse_prepare(document);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_html_parse_fragment_chunk_begin(parser, document,
                                               element->node.tag_id,
                                               element->node.ns);
}

lxb_status_t
lxb_html_document_parse_fragment_chunk(lxb_html_document_t *document,
                                       const lxb_char_t *html, size_t size)
{
    if (document->mem == NULL) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    return lxb_html_parse_fragment_chunk_process(document->mem->parser,
                                                 html, size);
}

lxb_dom_node_t *
lxb_html_document_parse_fragment_chunk_end(lxb_html_document_t *document)
{
    if (document->mem == NULL) {
        return NULL;
    }

    return lxb_html_parse_fragment_chunk_end(document->mem->parser);
}

lxb_inline lxb_status_t
lxb_html_document_parse_prepare(lxb_html_document_t *document)
{
    if (document->mem == NULL) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    if (document->mem->parser == NULL) {
        lxb_status_t status;

        document->mem->parser = lxb_html_parser_create();
        status = lxb_html_parser_init(document->mem->parser);

        if (status != LXB_STATUS_OK) {
            lxb_html_parser_destroy(document->mem->parser, true);

            return status;
        }
    }
    else {
        lxb_html_parser_clean(document->mem->parser);
    }

    if ((document->opt & LXB_HTML_DOCUMENT_PARSE_WO_COPY)) {
        lxb_html_parser_set_without_copy(document->mem->parser);
    }

    return LXB_STATUS_OK;
}

const lxb_char_t *
lxb_html_document_title(lxb_html_document_t *document, size_t *len)
{
    lxb_html_title_element_t *title = NULL;

    lxb_dom_node_simple_walk(lxb_dom_interface_node(document),
                             lxb_html_document_title_walker, &title);
    if (title == NULL) {
        return NULL;
    }

    return lxb_html_title_element_strict_text(title, len);
}

lxb_status_t
lxb_html_document_title_set(lxb_html_document_t *document,
                            const lxb_char_t *title, size_t len)
{
    lxb_status_t status;

    /* TODO: If the document element is an SVG svg element */

    /* If the document element is in the HTML namespace */
    if (document->head == NULL) {
        return LXB_STATUS_OK;
    }

    lxb_html_title_element_t *el_title = NULL;

    lxb_dom_node_simple_walk(lxb_dom_interface_node(document),
                             lxb_html_document_title_walker, &el_title);
    if (el_title == NULL) {
        el_title = (void *) lxb_html_document_create_element(document,
                                         (const lxb_char_t *) "title", 5, NULL);
        if (el_title == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        lxb_dom_node_insert_child(lxb_dom_interface_node(document->head),
                                  lxb_dom_interface_node(el_title));
    }

    status = lxb_dom_node_text_content_set(lxb_dom_interface_node(el_title),
                                           title, len);
    if (status != LXB_STATUS_OK) {
        lxb_html_document_destroy_element(&el_title->element.element);

        return status;
    }

    return LXB_STATUS_OK;
}

const lxb_char_t *
lxb_html_document_title_raw(lxb_html_document_t *document, size_t *len)
{
    lxb_html_title_element_t *title = NULL;

    lxb_dom_node_simple_walk(lxb_dom_interface_node(document),
                             lxb_html_document_title_walker, &title);
    if (title == NULL) {
        return NULL;
    }

    return lxb_html_title_element_text(title, len);
}

static lexbor_action_t
lxb_html_document_title_walker(lxb_dom_node_t *node, void *ctx)
{
    if (node->tag_id == LXB_TAG_TITLE) {
        *((void **) ctx) = node;

        return LEXBOR_ACTION_STOP;
    }

    return LEXBOR_ACTION_NEXT;
}

/*
 * No inline functions for ABI.
 */
lxb_html_head_element_t *
lxb_html_document_head_element_noi(lxb_html_document_t *document)
{
    return lxb_html_document_head_element(document);
}

lxb_html_body_element_t *
lxb_html_document_body_element_noi(lxb_html_document_t *document)
{
    return lxb_html_document_body_element(document);
}

lxb_dom_document_t *
lxb_html_document_original_ref_noi(lxb_html_document_t *document)
{
    return lxb_html_document_original_ref(document);
}

bool
lxb_html_document_is_original_noi(lxb_html_document_t *document)
{
    return lxb_html_document_is_original(document);
}

lexbor_mraw_t *
lxb_html_document_mraw_noi(lxb_html_document_t *document)
{
    return lxb_html_document_mraw(document);
}

lexbor_mraw_t *
lxb_html_document_mraw_text_noi(lxb_html_document_t *document)
{
    return lxb_html_document_mraw_text(document);
}

lxb_tag_heap_t *
lxb_html_document_tag_heap_noi(lxb_html_document_t *document)
{
    return lxb_html_document_tag_heap(document);
}

lxb_ns_heap_t *
lxb_html_document_ns_heap_noi(lxb_html_document_t *document)
{
    return lxb_html_document_ns_heap(document);
}

void
lxb_html_document_opt_set_noi(lxb_html_document_t *document,
                              lxb_html_document_opt_t opt)
{
    lxb_html_document_opt_set(document, opt);
}

lxb_html_document_opt_t
lxb_html_document_opt_noi(lxb_html_document_t *document)
{
    return lxb_html_document_opt(document);
}

void *
lxb_html_document_create_struct_noi(lxb_html_document_t *document,
                                    size_t struct_size)
{
    return lxb_html_document_create_struct(document, struct_size);
}

void *
lxb_html_document_destroy_struct_noi(lxb_html_document_t *document, void *data)
{
    return lxb_html_document_destroy_struct(document, data);
}

lxb_dom_element_t *
lxb_html_document_create_element_noi(lxb_html_document_t *document,
                                     const lxb_char_t *local_name,
                                     size_t lname_len, void *reserved_for_opt)
{
    return lxb_html_document_create_element(document, local_name, lname_len,
                                            reserved_for_opt);
}

lxb_dom_element_t *
lxb_html_document_destroy_element_noi(lxb_dom_element_t *element)
{
    return lxb_html_document_destroy_element(element);
}
