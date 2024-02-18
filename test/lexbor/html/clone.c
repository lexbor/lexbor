/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>

#include <unit/test.h>


const lxb_char_t html[] = "<div x=abc><span>darkness</span><xx>xXx</xx></div>";

const size_t html_length = sizeof(html) - 1;


TEST_BEGIN(single_clone)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr_cloned, *attr_orig;
    lxb_dom_node_t *node, *clone;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* Parse. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Get <div x="abc">. */

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    /* Clone <div x="abc">. */

    node = lxb_dom_collection_node(collection, 0);

    clone = lxb_dom_node_clone(node, 0);
    test_ne(clone, NULL);

    /* Insert cloned <div x="abc">. */

    lxb_dom_node_insert_after(node, clone);

    test_eq(node->next, clone);
    test_ne(node->next, node);

    /* Find all div tag. */

    collection->array.length = 0;

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 2);

    /* Get cloned attribute. */

    attr_cloned = lxb_dom_element_attr_by_name(lxb_dom_interface_element(clone),
                                               (lxb_char_t *) "x", 1);
    test_ne(attr_cloned, NULL);
    test_ne(attr_cloned->value, NULL);
    test_eq_str(attr_cloned->value->data, "abc");

    attr_orig = lxb_dom_element_attr_by_name(lxb_dom_interface_element(node),
                                             (lxb_char_t *) "x", 1);
    test_ne(attr_orig, NULL);
    test_ne(attr_orig->value, NULL);
    test_eq_str(attr_orig->value->data, "abc");

    test_ne(attr_cloned, attr_orig);
    test_ne(attr_cloned->value, attr_orig->value);
    test_ne(attr_cloned->value->data, attr_orig->value->data);

    test_eq_str(attr_cloned->value->data, attr_orig->value->data);
    test_eq(attr_cloned->value->length, attr_orig->value->length);

    /* Destroy. */

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(deep_clone)
{
    lxb_status_t status;
    lxb_dom_node_t *node, *clone, *span, *cloned_cpan;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* Parse. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Get <div x="abc">. */

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    /* Clone <div x="abc">. */

    node = lxb_dom_collection_node(collection, 0);

    clone = lxb_dom_node_clone(node, 1);
    test_ne(clone, NULL);

    /* Insert cloned <div x="abc">. */

    lxb_dom_node_insert_after(node, clone);

    test_eq(node->next, clone);
    test_ne(node->next, node);

    /* Find all span tag. */

    collection->array.length = 0;

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "span", 4);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 2);

    /* Check parents. */

    span = lxb_dom_collection_node(collection, 0);
    cloned_cpan = lxb_dom_collection_node(collection, 1);

    test_ne(span, NULL);
    test_ne(cloned_cpan, NULL);
    test_ne(span, cloned_cpan);

    test_eq(span->parent, node);
    test_eq(cloned_cpan->parent, clone);

    /* Destroy. */

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(text_clone)
{
    lxb_status_t status;
    lxb_dom_node_t *node, *clone;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* Parse. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Get <span>. */

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "span", 4);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    /* Clone #text. */

    node = lxb_dom_collection_node(collection, 0);

    clone = lxb_dom_node_clone(node->first_child, 0);
    test_ne(clone, NULL);

    /* Insert cloned #text. */

    lxb_dom_node_insert_after(node, clone);

    test_eq(node->next, clone);
    test_ne(node->next, node->first_child);

    /* Destroy. */

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(import_from)
{
    lxb_status_t status;
    lxb_dom_node_t *node_one, *node_two, *clone;
    lxb_html_document_t *document_one, *document_two;
    lxb_dom_collection_t *collection;

    /* Parse. */

    document_one = lxb_html_document_create();
    test_ne(document_one, NULL);

    status = lxb_html_document_parse(document_one, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    document_two = lxb_html_document_create();
    test_ne(document_two, NULL);

    status = lxb_html_document_parse(document_two, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Collection one. */

    collection = lxb_dom_collection_make(&document_one->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document_one),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    node_one = lxb_dom_collection_node(collection, 0);

    /* Collection two. */

    collection->array.length = 0;

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document_two),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    node_two = lxb_dom_collection_node(collection, 0);

    /* Check. */

    test_ne(node_one, node_two);

    /* Clone. */

    clone = lxb_dom_document_import_node(&document_one->dom_document,
                                         node_two, 1);
    test_ne(clone, NULL);

    test_eq(clone->owner_document, &document_one->dom_document);

    /* Destroy. */

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document_one);
    lxb_html_document_destroy(document_two);
}
TEST_END


TEST_BEGIN(full_clone)
{
    lxb_status_t status;
    lexbor_str_t str_one, str_two;
    lxb_dom_node_t *node_one, *node_two, *clone;
    lxb_html_document_t *document_one, *document_two;

    /* Parse. */

    document_one = lxb_html_document_create();
    test_ne(document_one, NULL);

    status = lxb_html_document_parse(document_one, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    document_two = lxb_html_document_create();
    test_ne(document_two, NULL);

    status = lxb_html_document_parse(document_two, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Root. */

    node_one = lxb_dom_document_root(lxb_dom_interface_document(document_one));
    node_two = lxb_dom_document_root(lxb_dom_interface_document(document_two));

    /* Check. */

    test_ne(node_one, node_two);

    /* Clone. */

    clone = lxb_dom_document_import_node(lxb_dom_interface_document(document_one),
                                         node_two, 1);
    test_ne(clone, NULL);

    test_eq(clone->owner_document, &document_one->dom_document);

    lexbor_str_clean_all(&str_one);
    lexbor_str_clean_all(&str_two);

    status = lxb_html_serialize_pretty_tree_str(node_one, 0, 0, &str_one);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_serialize_pretty_tree_str(clone, 0, 0, &str_two);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(str_one.data, str_one.length, str_two.data, str_two.length);

    /* Destroy. */

    lxb_html_document_destroy(document_one);
    lxb_html_document_destroy(document_two);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(single_clone);
    TEST_ADD(deep_clone);
    TEST_ADD(text_clone);
    TEST_ADD(import_from);
    TEST_ADD(full_clone);

    TEST_RUN("lexbor/html/clone");
    TEST_RELEASE();
}
