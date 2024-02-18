/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>

#include <unit/test.h>

TEST_BEGIN(text_node_without_parent)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_dom_text_t *text;
    lxb_html_document_t *document;

    static const lxb_char_t txt[] = "abc";
    static const size_t length = sizeof(txt) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    text = lxb_dom_document_create_text_node(&document->dom_document,
                                             txt, length);
    test_ne(text, NULL);

    status = lxb_html_serialize_str(lxb_dom_interface_node(text), &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, length);
    test_eq_str(str.data, txt);

    str.length = 0;

    status = lxb_html_serialize_pretty_str(lxb_dom_interface_node(text), 0, 0,
                                           &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, length + 3); /* "abc\n" */
    test_eq_str(str.data, "\"abc\"\n");

    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(text_node_without_parent);

    TEST_RUN("lexbor/html/serialize");
    TEST_RELEASE();
}
