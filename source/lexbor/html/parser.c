/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/parser.h"
#include "lexbor/html/tree/open_elements.h"
#include "lexbor/html/interfaces/element.h"
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
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_document_t *own_doc;
    lxb_html_tree_t *original_tree;
    lxb_dom_node_t *root;
    lxb_dom_node_t *el_node = lxb_dom_interface_node(element);

    own_doc = lxb_dom_interface_node(element)->owner_document;

    document = lxb_html_document_create(NULL);
    status = lxb_html_document_init(document, parser->tag_heap);

    if (status != LXB_STATUS_OK) {
        parser->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

        return NULL;
    }

    document->dom_document.compat_mode = own_doc->compat_mode;

    lxb_html_tokenizer_set_state_by_tag(parser->tkz,
                                        document->dom_document.scripting,
                                        el_node->tag_id, el_node->ns);

    root = lxb_html_tree_create_node(parser->tree, LXB_HTML_TAG_HTML,
                                     LXB_HTML_NS_HTML);
    if (root == NULL) {
        goto failed;
    }

    parser->tree->fragment = root;

    lxb_dom_node_insert_child(lxb_dom_interface_node(document), root);
    lxb_dom_document_attach_element(&document->dom_document,
                                    lxb_dom_interface_element(root));

    /* Contains just the single element root. */
    parser->status = lxb_html_tree_open_elements_push(parser->tree, root);
    if (parser->status != LXB_STATUS_OK) {
        goto failed;
    }

    if (lxb_html_tree_node_is(el_node, LXB_HTML_TAG_TEMPLATE)) {
        parser->status = lxb_html_tree_template_insertion_push(parser->tree,
                                                               lxb_html_tree_insertion_mode_in_template);
        if (parser->status != LXB_STATUS_OK) {
            goto failed;
        }
    }

    lxb_html_tree_reset_insertion_mode_appropriately(parser->tree);

    if (lxb_html_tree_node_is(el_node, LXB_HTML_TAG_FORM)) {
        parser->tree->form = lxb_html_interface_form(el_node);
    }

    original_tree = lxb_html_tokenizer_tree(parser->tkz);
    lxb_html_tokenizer_tree_set(parser->tkz, parser->tree);

    parser->status = lxb_html_tree_build(parser->tree, document, html, size);
    if (parser->status != LXB_STATUS_OK) {
        document = lxb_html_document_destroy(document);
    }

    lxb_html_tokenizer_tree_set(parser->tkz, original_tree);

    return root->first_child;

failed:

    lxb_html_document_destroy(document);

    return NULL;
}
