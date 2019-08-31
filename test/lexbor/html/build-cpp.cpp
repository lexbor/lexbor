/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <iostream>

#include <unit/test.h>

#include <lexbor/html/interfaces/document.h>
#include <lexbor/html/serialize.h>


TEST_BEGIN(parse)
{
    lxb_status_t status;
    lxb_html_document_t *document;

    const lxb_char_t html[] = "<div>V</div>";
    size_t html_len = sizeof(html) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_len);
    test_eq(status, LXB_STATUS_OK);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(serialize)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_document_t *document;
    lxb_html_body_element_t *body;

    const lxb_char_t html[] = "<div>V</div>";
    size_t html_len = sizeof(html) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_len);
    test_eq(status, LXB_STATUS_OK);

    body = lxb_html_document_body_element(document);

    status = lxb_html_serialize_str(lxb_dom_interface_node(body), &str);
    test_eq(status, LXB_STATUS_OK);

    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(parse);
    TEST_ADD(serialize);

    TEST_RUN("lexbor/html/build-cpp");
    TEST_RELEASE();
}
