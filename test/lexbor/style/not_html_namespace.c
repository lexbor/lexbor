/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/style/style.h>
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

TEST_BEGIN(styles)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_html_element_t *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* HTML Data. */

    static const lexbor_str_t html = lexbor_str("<style>.father {width: 120px}</style>"
                                                "<div class=father><svg style='width: 10em'>"
                                                "<style>.father {height: 380px}</style>"
                                                "<path style='width: 50%' class=father></svg></div>");

    /* Other Data. */

    static const lexbor_str_t father_str = lexbor_str("father");
    static const lexbor_str_t result_str = lexbor_str("width: 120px");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    /* Init all CSS objects and momory for Document. */

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    /* Parse HTML. */

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    /* Find HTML nodes by CSS Selectors. */

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_class_name(lxb_dom_interface_node(document),
                                        collection, father_str.data,
                                        father_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    div = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Tests 1. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, result_str.data, result_str.length);

    /* Destroy resources. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

TEST_BEGIN_ARGS(foreign_option_selected, const lxb_char_t *ns,
                lxb_dom_document_opt_t append_opt,
                lxb_dom_document_opt_t change_opt)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_element_t *option;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    dom_document = lxb_dom_interface_document(document);

    status = mraw_chunk_fill(dom_document->mraw, sizeof(lxb_dom_element_t));
    test_eq(status, LXB_STATUS_OK);

    option = lxb_dom_element_create(dom_document, option_str,
                                    sizeof(option_str) - 1,
                                    ns, strlen((const char *) ns),
                                    NULL, 0, NULL, 0, false);
    test_ne(option, NULL);
    test_ne(option->node.ns, LXB_NS_HTML);
    test_eq(mraw_chunk_end(dom_document->mraw, option,
                           sizeof(lxb_dom_element_t)), true);

    lxb_dom_document_opt_set(dom_document, append_opt);

    attr = lxb_dom_element_set_attribute(option, selected_str,
                                         sizeof(selected_str) - 1,
                                         (const lxb_char_t *) "", 0);
    test_ne(attr, NULL);

    lxb_dom_document_opt_set(dom_document, change_opt);

    attr = lxb_dom_element_set_attribute(option, selected_str,
                                         sizeof(selected_str) - 1,
                                         (const lxb_char_t *) "selected", 8);
    test_ne(attr, NULL);

    lxb_dom_document_opt_set(dom_document, LXB_DOM_DOCUMENT_OPT_UNDEF);

    status = lxb_dom_element_remove_attribute(option, selected_str,
                                              sizeof(selected_str) - 1);
    test_eq(status, LXB_STATUS_OK);

    lxb_dom_element_destroy(option);

    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(svg_option_append)
{
    TEST_CALL_ARGS(foreign_option_selected, svg_ns,
                   LXB_DOM_DOCUMENT_OPT_UNDEF, LXB_DOM_DOCUMENT_OPT_UNDEF);
}
TEST_END

TEST_BEGIN(mathml_option_append)
{
    TEST_CALL_ARGS(foreign_option_selected, mathml_ns,
                   LXB_DOM_DOCUMENT_OPT_UNDEF, LXB_DOM_DOCUMENT_OPT_UNDEF);
}
TEST_END

TEST_BEGIN(svg_option_change)
{
    TEST_CALL_ARGS(foreign_option_selected, svg_ns,
                   LXB_DOM_DOCUMENT_OPT_WO_EVENTS, LXB_DOM_DOCUMENT_OPT_UNDEF);
}
TEST_END

TEST_BEGIN(mathml_option_change)
{
    TEST_CALL_ARGS(foreign_option_selected, mathml_ns,
                   LXB_DOM_DOCUMENT_OPT_WO_EVENTS, LXB_DOM_DOCUMENT_OPT_UNDEF);
}
TEST_END

TEST_BEGIN(svg_option_remove)
{
    TEST_CALL_ARGS(foreign_option_selected, svg_ns,
                   LXB_DOM_DOCUMENT_OPT_WO_EVENTS,
                   LXB_DOM_DOCUMENT_OPT_WO_EVENTS);
}
TEST_END

TEST_BEGIN(mathml_option_remove)
{
    TEST_CALL_ARGS(foreign_option_selected, mathml_ns,
                   LXB_DOM_DOCUMENT_OPT_WO_EVENTS,
                   LXB_DOM_DOCUMENT_OPT_WO_EVENTS);
}
TEST_END

TEST_BEGIN(svg_option_inline_style)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_node_t *node;
    lxb_dom_element_t *option;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<svg><option selected "
                                                "style='width: 10em'>");
    static const lexbor_str_t result_str = lexbor_str("width: 10em");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    node = lxb_dom_interface_node(document->body)->first_child;
    test_ne(node, NULL);

    node = node->first_child;
    test_ne(node, NULL);
    test_eq(node->local_name, LXB_TAG_OPTION);
    test_eq(node->ns, LXB_NS_SVG);

    option = lxb_dom_interface_element(node);

    out.data = NULL;
    status = lxb_dom_element_style_serialize_str(option, &out,
                                                 LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, result_str.data, result_str.length);

    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(styles);
    TEST_ADD(svg_option_append);
    TEST_ADD(mathml_option_append);
    TEST_ADD(svg_option_change);
    TEST_ADD(mathml_option_change);
    TEST_ADD(svg_option_remove);
    TEST_ADD(mathml_option_remove);
    TEST_ADD(svg_option_inline_style);

    TEST_RUN("lexbor/style/not_html_namespace");
    TEST_RELEASE();
}
