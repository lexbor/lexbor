/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/parser.h"
#include "lexbor/html/html.h"
#include "lexbor/html/tree/open_elements.h"
#include "lexbor/html/interfaces/element.h"
#include "lexbor/html/interfaces/html_element.h"
#include "lexbor/html/interfaces/form_element.h"
#include "lexbor/html/tree/template_insertion.h"
#include "lexbor/html/tree/insertion_mode.h"


lxb_html_parser_t *
lxb_html_parser_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_html_parser_t));
}

lxb_status_t
lxb_html_parser_init(lxb_html_parser_t *parser)
{
    if (parser == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    /* Tokenizer */
    parser->tkz = lxb_html_tokenizer_create();
    lxb_status_t status = lxb_html_tokenizer_init(parser->tkz);

    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Tag Heap for Tokenizer and Document */
    parser->tag_heap = lxb_html_tag_heap_create();

    status = lxb_html_tag_heap_init(parser->tag_heap, 128);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    lxb_html_tokenizer_tag_heap_set(parser->tkz, parser->tag_heap);

    /* Tree */
    parser->tree = lxb_html_tree_create();
    status = lxb_html_tree_init(parser->tree, parser->tkz);

    if (status != LXB_STATUS_OK) {
        return status;
    }

    return LXB_STATUS_OK;
}

void
lxb_html_parser_clean(lxb_html_parser_t *parser)
{
    lxb_html_tokenizer_clean(parser->tkz);
    lxb_html_tag_heap_clean(parser->tag_heap);
    lxb_html_tree_clean(parser->tree);
}

lxb_html_parser_t *
lxb_html_parser_destroy(lxb_html_parser_t *parser, bool self_destroy)
{
    if (parser == NULL) {
        return NULL;
    }

    parser->tkz = lxb_html_tokenizer_unref(parser->tkz, true);
    parser->tag_heap = lxb_html_tag_heap_unref(parser->tag_heap, true);
    parser->tree = lxb_html_tree_unref(parser->tree, true);

    return parser;
}

lxb_html_document_t *
lxb_html_parse(lxb_html_parser_t *parser, const lxb_char_t *html, size_t size)
{
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_html_tree_t *original_tree;

    document = lxb_html_document_create(NULL);
    status = lxb_html_document_init(document, parser->tag_heap);

    if (status != LXB_STATUS_OK) {
        parser->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

        return NULL;
    }

    document->dom_document.scripting = parser->tree->scripting;

    original_tree = lxb_html_tokenizer_tree(parser->tkz);
    lxb_html_tokenizer_tree_set(parser->tkz, parser->tree);

    parser->status = lxb_html_tree_build(parser->tree, document, html, size);
    if (parser->status != LXB_STATUS_OK) {
        document = lxb_html_document_destroy(document);
    }

    lxb_html_tokenizer_tree_set(parser->tkz, original_tree);

    return document;
}

lxb_dom_node_t *
lxb_html_parse_fragment(lxb_html_parser_t *parser, lxb_html_element_t *element,
                        const lxb_char_t *html, size_t size)
{
    return lxb_html_parse_fragment_by_tag_id(parser,
                                             parser->tree->document,
                                             element->element.node.tag_id,
                                             element->element.node.ns,
                                             html, size);
}

lxb_dom_node_t *
lxb_html_parse_fragment_by_tag_id(lxb_html_parser_t *parser,
                                  lxb_html_document_t *document,
                                  lxb_html_tag_id_t tag_id, lxb_html_ns_id_t ns,
                                  const lxb_char_t *html, size_t size)
{
    lxb_status_t status;
    lxb_html_document_t *new_doc;
    lxb_html_tree_t *original_tree;
    lxb_dom_node_t *root, *fragment, *form = NULL;

    new_doc = lxb_html_document_create(document);
    status = lxb_html_document_init(new_doc, parser->tag_heap);

    if (status != LXB_STATUS_OK) {
        parser->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

        return NULL;
    }

    if (document != NULL) {
        new_doc->dom_document.scripting = document->dom_document.scripting;
        new_doc->dom_document.compat_mode = document->dom_document.compat_mode;
    }
    else {
        new_doc->dom_document.scripting = parser->tree->scripting;
        new_doc->dom_document.compat_mode = LXB_DOM_DOCUMENT_CMODE_NO_QUIRKS;
    }

    lxb_html_tokenizer_set_state_by_tag(parser->tkz,
                                        new_doc->dom_document.scripting,
                                        tag_id, ns);

    root = lxb_html_create_node(new_doc, LXB_HTML_TAG_HTML,
                                LXB_HTML_NS_HTML);
    if (root == NULL) {
        parser->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

        goto done;
    }

    lxb_dom_node_insert_child(lxb_dom_interface_node(new_doc), root);
    lxb_dom_document_attach_element(&new_doc->dom_document,
                                    lxb_dom_interface_element(root));

    fragment = lxb_html_create_node(new_doc, tag_id, ns);
    if (fragment == NULL) {
        parser->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

        goto done;
    }

    parser->tree->fragment = fragment;

    /* Contains just the single element root. */
    parser->status = lxb_html_tree_open_elements_push(parser->tree, root);
    if (parser->status != LXB_STATUS_OK) {
        goto done;
    }

    if (tag_id == LXB_HTML_TAG_TEMPLATE && ns == LXB_HTML_NS_HTML) {
        parser->status = lxb_html_tree_template_insertion_push(parser->tree,
                                                               lxb_html_tree_insertion_mode_in_template);
        if (parser->status != LXB_STATUS_OK) {
            goto done;
        }
    }

    lxb_html_tree_attach_document(parser->tree, new_doc);
    lxb_html_tree_reset_insertion_mode_appropriately(parser->tree);

    if (tag_id == LXB_HTML_TAG_FORM && ns == LXB_HTML_NS_HTML) {
        form = lxb_html_create_node(new_doc, LXB_HTML_TAG_FORM,
                                    LXB_HTML_NS_HTML);
        if (form == NULL) {
            parser->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

            goto done;
        }

        parser->tree->form = lxb_html_interface_form(form);
    }

    original_tree = lxb_html_tokenizer_tree(parser->tkz);
    lxb_html_tokenizer_tree_set(parser->tkz, parser->tree);

    parser->status = lxb_html_tree_build(parser->tree, new_doc, html, size);

    lxb_html_tokenizer_tree_set(parser->tkz, original_tree);

done:

    if (parser->status != LXB_STATUS_OK) {
        lxb_html_html_element_destroy(lxb_html_interface_html(root));

        root = NULL;
    }

    if (form != NULL) {
        lxb_html_form_element_destroy(lxb_html_interface_form(form));
    }

    /*
     * Here everything is true!
     * If we are inherited then we delete the unnecessary document.
     */
    if (document != NULL) {
        lxb_html_document_destroy(new_doc);
    }

    return root;
}
