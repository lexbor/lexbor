/*
 * Copyright (C) 2022-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>

#include <unit/test.h>

static lexbor_action_t
remove_attrs(lxb_dom_node_t *node, void *ctx)
{
    lxb_dom_element_t *elem;
    lxb_dom_attr_t *attr, *next;

    if (node->type != LXB_DOM_NODE_TYPE_ELEMENT) {
        return LEXBOR_ACTION_OK;
    }

    elem = lxb_dom_interface_element(node);
    attr = lxb_dom_element_first_attribute(elem);

    while (attr) {
        next = lxb_dom_element_next_attribute(attr);

        lxb_dom_attr_remove(attr);
        lxb_dom_attr_interface_destroy(attr);

        attr = next;
    }

    return LEXBOR_ACTION_OK;
}

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

TEST_BEGIN(bad_html_remove_attributes)
{
    lxb_status_t status;
    lexbor_str_t str;
    lxb_dom_node_t *root;
    lxb_html_document_t *document;

    static const lxb_char_t html[] = "<a target=\"x\"><div></a>";
    static const lxb_char_t res[] = "<html><head></head><body><a></a>"
                                    "<div><a></a></div></body></html>";

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, sizeof(html));
    test_eq(status, LXB_STATUS_OK);

    root = lxb_dom_document_root(lxb_dom_interface_document(document));

    lxb_dom_node_simple_walk(root, remove_attrs, NULL);

    str.data = NULL;
    lxb_html_serialize_tree_str(root, &str);

    test_eq(str.length, sizeof(res) - 1);
    test_eq_str(str.data, res);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(duplicate_attributes_svg_namespace)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_document_t *document;

    static const lxb_char_t html[] = "<svg>"
    "<use xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
    "xmlns:xlink=\"http://www.w3.org/1999/xlink\">";
    static const size_t length = sizeof(html) - 1;

    static const lxb_char_t res[] = "<svg>"
    "<use xmlns:xlink=\"http://www.w3.org/1999/xlink\"></use></svg>";
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
    TEST_ADD(bad_html_remove_attributes);
    TEST_ADD(duplicate_attributes_svg_namespace);

    TEST_RUN("lexbor/html/other");
    TEST_RELEASE();
}
