/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>

#include <unit/test.h>

TEST_BEGIN(fixed_svg_tags)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_document_t *document;

    static const lxb_char_t html[] = "<svg><a xlink:title>";
    static const size_t length = sizeof(html) - 1;

    static const lxb_char_t res[] = "<svg><a xlink:title></a></svg>";
    static const size_t res_length = sizeof(res) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_serialize_deep_str(lxb_dom_interface_node(document->body),
                                         &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, res_length); /* "abc\n" */
    test_eq_str(str.data, res);

    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(fixed_svg_tags);

    TEST_RUN("lexbor/html/other");
    TEST_RELEASE();
}
