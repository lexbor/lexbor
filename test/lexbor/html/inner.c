/*
 * Copyright (C) 2024 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>

#include <unit/test.h>


typedef struct {
    const char *html;
    const char *inner;
    const char *result;
}
lxb_test_data_t;


static const lxb_test_data_t lxb_data[] =
{
    {
        "<body><textarea>contents</textarea>",
        "</textarea>foo",
        "<textarea>&lt;/textarea&gt;foo</textarea>"
    },
    {
        "<body><style>contents</style>",
        "</style>foo",
        "<style></style>foo</style>"
    },
    {
        "<body><script>contents</script>",
        "</script>foo",
        "<script></script>foo</script>"
    },
    {
        "<body><plaintext>contents</plaintext>",
        "</plaintext>foo",
        "<plaintext></plaintext>foo</plaintext>"
    },
    {NULL, NULL, NULL}
};


TEST_BEGIN(inner_special_tags)
{
    lxb_status_t status;
    lexbor_str_t str;
    lxb_dom_node_t *node;
    lxb_html_element_t *element;
    lxb_html_document_t *document;
    lxb_html_body_element_t *body;
    const lxb_test_data_t *test;

    test = lxb_data;

    while (test->html != NULL) {
        TEST_PRINTLN("Test %ld", (long) (test - lxb_data));

        document = lxb_html_document_create();
        test_ne(document, NULL);

        status = lxb_html_document_parse(document, (const lxb_char_t *) test->html,
                                         strlen(test->html));
        test_eq(status, LXB_STATUS_OK);

        body = lxb_html_document_body_element(document);
        test_ne(body, NULL);

        node = lxb_dom_node_first_child(lxb_dom_interface_node(body));
        test_ne(node, NULL);

        element = lxb_html_element_inner_html_set(lxb_html_interface_element(node),
                                                  (const lxb_char_t *) test->inner,
                                                  strlen(test->inner));
        test_ne(element, NULL);

        str.data = NULL;
        node = lxb_dom_node_first_child(lxb_dom_interface_node(body));

        status = lxb_html_serialize_tree_str(node, &str);
        test_eq(status, LXB_STATUS_OK);

        test_eq_str(str.data, test->result);

        lxb_html_document_destroy(document);

        test += 1;
    }
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(inner_special_tags);

    TEST_RUN("lexbor/html/inner");
    TEST_RELEASE();
}
