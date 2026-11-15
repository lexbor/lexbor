/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/html/html.h>


static const lxb_char_t svg_ns[] = "http://www.w3.org/2000/svg";

static const lxb_char_t rect_str[] = "rect";
static const lxb_char_t svg_str[] = "svg";


TEST_BEGIN(prefix)
{
    size_t len;
    const lxb_char_t *prefix;
    lxb_dom_element_t *element;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);

    element = lxb_dom_element_create(dom_document, rect_str,
                                     sizeof(rect_str) - 1,
                                     svg_ns, sizeof(svg_ns) - 1,
                                     svg_str, sizeof(svg_str) - 1,
                                     NULL, 0, false);
    test_ne(element, NULL);

    len = SIZE_MAX;

    prefix = lxb_dom_element_prefix(element, &len);
    test_ne(prefix, NULL);
    test_eq(len, sizeof(svg_str) - 1);
    test_eq_str_n(prefix, len, svg_str, sizeof(svg_str) - 1);

    lxb_dom_element_destroy(element);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(prefix_without)
{
    size_t len;
    const lxb_char_t *prefix;
    lxb_dom_element_t *element;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);

    element = lxb_dom_element_create(dom_document, rect_str,
                                     sizeof(rect_str) - 1,
                                     svg_ns, sizeof(svg_ns) - 1,
                                     NULL, 0, NULL, 0, false);
    test_ne(element, NULL);

    len = SIZE_MAX;

    prefix = lxb_dom_element_prefix(element, &len);
    test_eq(prefix, NULL);
    test_eq(len, 0);

    lxb_dom_element_destroy(element);
    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(prefix);
    TEST_ADD(prefix_without);

    TEST_RUN("lexbor/dom/element");
    TEST_RELEASE();
}
