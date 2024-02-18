/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/html/tree.h>
#include <lexbor/html/tree/open_elements.h>


TEST_BEGIN(remove_by_node)
{
    /* Tokenizer */
    lxb_html_tokenizer_t *tkz = lxb_html_tokenizer_create();
    lxb_status_t status = lxb_html_tokenizer_init(tkz);

    test_eq(status, LXB_STATUS_OK);

    /* Tree */
    lxb_html_tree_t *tree = lxb_html_tree_create();
    status = lxb_html_tree_init(tree, tkz);

    test_eq(status, LXB_STATUS_OK);

    /* Test */
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 1), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 2), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 3), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 4), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 5), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 6), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 7), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 8), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 9), LXB_STATUS_OK);

    lxb_html_tree_open_elements_remove_by_node(tree, (void *) 4);

    test_eq(lxb_html_tree_open_elements_get(tree, 0), (void *) 1);
    test_eq(lxb_html_tree_open_elements_get(tree, 1), (void *) 2);
    test_eq(lxb_html_tree_open_elements_get(tree, 2), (void *) 3);
    test_eq(lxb_html_tree_open_elements_get(tree, 3), (void *) 5);
    test_eq(lxb_html_tree_open_elements_get(tree, 4), (void *) 6);
    test_eq(lxb_html_tree_open_elements_get(tree, 5), (void *) 7);
    test_eq(lxb_html_tree_open_elements_get(tree, 6), (void *) 8);
    test_eq(lxb_html_tree_open_elements_get(tree, 7), (void *) 9);

    test_eq_size(lexbor_array_length(tree->open_elements), 8UL);

    lxb_html_tokenizer_unref(tkz);
    lxb_html_tree_unref(tree);
}
TEST_END

TEST_BEGIN(remove_by_node_one)
{
    /* Tokenizer */
    lxb_html_tokenizer_t *tkz = lxb_html_tokenizer_create();
    lxb_status_t status = lxb_html_tokenizer_init(tkz);

    test_eq(status, LXB_STATUS_OK);

    /* Tree */
    lxb_html_tree_t *tree = lxb_html_tree_create();
    status = lxb_html_tree_init(tree, tkz);

    test_eq(status, LXB_STATUS_OK);

    /* Test */
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 1), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 2), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 3), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, (void *) 4), LXB_STATUS_OK);

    lxb_html_tree_open_elements_remove_by_node(tree, (void *) 4);

    test_eq(lxb_html_tree_open_elements_get(tree, 0), (void *) 1);
    test_eq(lxb_html_tree_open_elements_get(tree, 1), (void *) 2);
    test_eq(lxb_html_tree_open_elements_get(tree, 2), (void *) 3);

    test_eq_size(lexbor_array_length(tree->open_elements), 3UL);

    lxb_html_tokenizer_unref(tkz);
    lxb_html_tree_unref(tree);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(remove_by_node);
    TEST_ADD(remove_by_node_one);

    TEST_RUN("lexbor/html/tree/open_elements");
    TEST_RELEASE();
}
