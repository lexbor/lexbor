/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/core/mraw.h>

#include <unit/test.h>


static const lxb_char_t svg_ns[] = "http://www.w3.org/2000/svg";
static const lxb_char_t mathml_ns[] = "http://www.w3.org/1998/Math/MathML";

static const lxb_char_t option_str[] = "option";
static const lxb_char_t selected_str[] = "selected";


static lxb_status_t
mraw_chunk_fill(lexbor_mraw_t *mraw, size_t size)
{
    size_t meta, need, rest;
    lexbor_mem_chunk_t *chunk;

    meta = lexbor_mraw_meta_size();
    need = lexbor_mem_align(size) + meta;

    chunk = mraw->mem->chunk;
    rest = chunk->size - chunk->length;

    if (rest < need + meta + LEXBOR_MEM_ALIGN_STEP) {
        if (rest > meta && lexbor_mraw_alloc(mraw, rest - meta) == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        if (lexbor_mraw_alloc(mraw, LEXBOR_MEM_ALIGN_STEP) == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        chunk = mraw->mem->chunk;
        rest = chunk->size - chunk->length;
    }

    if (lexbor_mraw_alloc(mraw, rest - need - meta) == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

static bool
mraw_chunk_end(lexbor_mraw_t *mraw, void *data, size_t size)
{
    uintptr_t begin, end;
    lexbor_mem_chunk_t *chunk;

    begin = (uintptr_t) data;
    end = begin + lexbor_mem_align(size);

    chunk = mraw->mem->chunk;

    while (chunk != NULL) {
        if (begin >= (uintptr_t) chunk->data
            && end <= (uintptr_t) chunk->data + chunk->size)
        {
            return end == (uintptr_t) chunk->data + chunk->size;
        }

        chunk = chunk->prev;
    }

    return false;
}

static lxb_dom_element_t *
foreign_option_create(lxb_dom_document_t *document, const lxb_char_t *ns)
{
    return lxb_dom_element_create(document, option_str, sizeof(option_str) - 1,
                                  ns, strlen((const char *) ns),
                                  NULL, 0, NULL, 0, false);
}

TEST_BEGIN_ARGS(foreign_option_append, const lxb_char_t *ns)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_element_t *option;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);

    status = mraw_chunk_fill(dom_document->mraw, sizeof(lxb_dom_element_t));
    test_eq(status, LXB_STATUS_OK);

    option = foreign_option_create(dom_document, ns);
    test_ne(option, NULL);
    test_ne(option->node.ns, LXB_NS_HTML);
    test_eq(mraw_chunk_end(dom_document->mraw, option,
                           sizeof(lxb_dom_element_t)), true);

    attr = lxb_dom_element_set_attribute(option, selected_str,
                                         sizeof(selected_str) - 1,
                                         (const lxb_char_t *) "", 0);
    test_ne(attr, NULL);

    lxb_dom_element_destroy(option);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN_ARGS(foreign_option_change, const lxb_char_t *ns)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_element_t *option;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);

    status = mraw_chunk_fill(dom_document->mraw, sizeof(lxb_dom_element_t));
    test_eq(status, LXB_STATUS_OK);

    option = foreign_option_create(dom_document, ns);
    test_ne(option, NULL);
    test_eq(mraw_chunk_end(dom_document->mraw, option,
                           sizeof(lxb_dom_element_t)), true);

    lxb_dom_document_opt_set(dom_document, LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    attr = lxb_dom_element_set_attribute(option, selected_str,
                                         sizeof(selected_str) - 1,
                                         (const lxb_char_t *) "", 0);
    test_ne(attr, NULL);

    lxb_dom_document_opt_set(dom_document, LXB_DOM_DOCUMENT_OPT_UNDEF);

    attr = lxb_dom_element_set_attribute(option, selected_str,
                                         sizeof(selected_str) - 1,
                                         (const lxb_char_t *) "selected", 8);
    test_ne(attr, NULL);

    lxb_dom_element_destroy(option);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN_ARGS(foreign_option_remove, const lxb_char_t *ns)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_element_t *option;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);

    status = mraw_chunk_fill(dom_document->mraw, sizeof(lxb_dom_element_t));
    test_eq(status, LXB_STATUS_OK);

    option = foreign_option_create(dom_document, ns);
    test_ne(option, NULL);
    test_eq(mraw_chunk_end(dom_document->mraw, option,
                           sizeof(lxb_dom_element_t)), true);

    lxb_dom_document_opt_set(dom_document, LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    attr = lxb_dom_element_set_attribute(option, selected_str,
                                         sizeof(selected_str) - 1,
                                         (const lxb_char_t *) "", 0);
    test_ne(attr, NULL);

    lxb_dom_document_opt_set(dom_document, LXB_DOM_DOCUMENT_OPT_UNDEF);

    status = lxb_dom_element_remove_attribute(option, selected_str,
                                              sizeof(selected_str) - 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_dom_element_has_attributes(option), false);

    lxb_dom_element_destroy(option);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN_ARGS(foreign_option_clone, const lxb_char_t *ns)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_node_t *copy;
    lxb_dom_element_t *option;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);

    option = foreign_option_create(dom_document, ns);
    test_ne(option, NULL);

    lxb_dom_document_opt_set(dom_document, LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    attr = lxb_dom_element_set_attribute(option, selected_str,
                                         sizeof(selected_str) - 1,
                                         (const lxb_char_t *) "", 0);
    test_ne(attr, NULL);

    lxb_dom_document_opt_set(dom_document, LXB_DOM_DOCUMENT_OPT_UNDEF);

    status = mraw_chunk_fill(dom_document->mraw, sizeof(lxb_dom_element_t));
    test_eq(status, LXB_STATUS_OK);

    copy = lxb_dom_node_clone(lxb_dom_interface_node(option), false);
    test_ne(copy, NULL);
    test_eq(copy->ns, option->node.ns);
    test_eq(mraw_chunk_end(dom_document->mraw, copy,
                           sizeof(lxb_dom_element_t)), true);
    test_eq(lxb_dom_element_has_attribute(lxb_dom_interface_element(copy),
                                          selected_str,
                                          sizeof(selected_str) - 1), true);

    lxb_dom_node_destroy(copy);
    lxb_dom_element_destroy(option);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN_ARGS(foreign_option_parse, const char *root,
                const lxb_char_t *ns)
{
    lxb_status_t status;
    lxb_dom_node_t *node;
    lxb_dom_document_t *dom_document;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    static const char option[] = "<option selected>";

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    document = lxb_html_parse_chunk_begin(parser);
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);

    status = lxb_html_parse_chunk_process(parser, (const lxb_char_t *) root,
                                          strlen(root));
    test_eq(status, LXB_STATUS_OK);

    status = mraw_chunk_fill(dom_document->mraw, sizeof(lxb_dom_element_t));
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_parse_chunk_process(parser, (const lxb_char_t *) option,
                                          sizeof(option) - 1);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_parse_chunk_end(parser);
    test_eq(status, LXB_STATUS_OK);

    node = lxb_dom_interface_node(document->body)->first_child;
    test_ne(node, NULL);

    node = node->first_child;
    test_ne(node, NULL);
    test_eq(node->local_name, LXB_TAG_OPTION);
    test_eq(node->ns, lxb_ns_data_by_link(dom_document->ns, ns,
                                          strlen((const char *) ns))->ns_id);
    test_eq(mraw_chunk_end(dom_document->mraw, node,
                           sizeof(lxb_dom_element_t)), true);

    lxb_html_document_destroy(document);
    lxb_html_parser_destroy(parser);
}
TEST_END

TEST_BEGIN(svg_option_append)
{
    TEST_CALL_ARGS(foreign_option_append, svg_ns);
}
TEST_END

TEST_BEGIN(mathml_option_append)
{
    TEST_CALL_ARGS(foreign_option_append, mathml_ns);
}
TEST_END

TEST_BEGIN(svg_option_change)
{
    TEST_CALL_ARGS(foreign_option_change, svg_ns);
}
TEST_END

TEST_BEGIN(mathml_option_change)
{
    TEST_CALL_ARGS(foreign_option_change, mathml_ns);
}
TEST_END

TEST_BEGIN(svg_option_remove)
{
    TEST_CALL_ARGS(foreign_option_remove, svg_ns);
}
TEST_END

TEST_BEGIN(mathml_option_remove)
{
    TEST_CALL_ARGS(foreign_option_remove, mathml_ns);
}
TEST_END

TEST_BEGIN(svg_option_clone)
{
    TEST_CALL_ARGS(foreign_option_clone, svg_ns);
}
TEST_END

TEST_BEGIN(mathml_option_clone)
{
    TEST_CALL_ARGS(foreign_option_clone, mathml_ns);
}
TEST_END

TEST_BEGIN(svg_option_parse)
{
    TEST_CALL_ARGS(foreign_option_parse, "<svg>", svg_ns);
}
TEST_END

TEST_BEGIN(mathml_option_parse)
{
    TEST_CALL_ARGS(foreign_option_parse, "<math>", mathml_ns);
}
TEST_END

TEST_BEGIN(html_option_selectedness)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_element_t *element;
    lxb_html_element_t *html_element;
    lxb_html_option_element_t *option;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    html_element = lxb_html_document_create_element(document, option_str,
                                                    sizeof(option_str) - 1,
                                                    NULL);
    test_ne(html_element, NULL);

    element = lxb_dom_interface_element(html_element);
    option = lxb_html_interface_option(html_element);
    test_eq(lxb_html_option_element_selectedness(option), false);

    attr = lxb_dom_element_set_attribute(element, selected_str,
                                         sizeof(selected_str) - 1,
                                         (const lxb_char_t *) "", 0);
    test_ne(attr, NULL);
    test_eq(lxb_html_option_element_selectedness(option), true);

    status = lxb_dom_element_remove_attribute(element, selected_str,
                                              sizeof(selected_str) - 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_html_option_element_selectedness(option), false);

    lxb_html_document_destroy_element(element);
    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(svg_option_append);
    TEST_ADD(mathml_option_append);
    TEST_ADD(svg_option_change);
    TEST_ADD(mathml_option_change);
    TEST_ADD(svg_option_remove);
    TEST_ADD(mathml_option_remove);
    TEST_ADD(svg_option_clone);
    TEST_ADD(mathml_option_clone);
    TEST_ADD(svg_option_parse);
    TEST_ADD(mathml_option_parse);
    TEST_ADD(html_option_selectedness);

    TEST_RUN("lexbor/html/attribute_steps");
    TEST_RELEASE();
}
