/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/avl.h>


TEST_BEGIN(init)
{
    lexbor_avl_t *avl = lexbor_avl_create();
    lxb_status_t status = lexbor_avl_init(avl, 1024);

    test_eq(status, LXB_STATUS_OK);

    lexbor_avl_destroy(avl, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_avl_init(NULL, 1024);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_avl_t avl;
    lxb_status_t status = lexbor_avl_init(&avl, 1024);

    test_eq(status, LXB_STATUS_OK);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_avl_t avl = {0};
    lxb_status_t status = lexbor_avl_init(&avl, 0);

    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(node_make)
{
    lexbor_avl_t avl;
    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_node_t *node = lexbor_avl_node_make(&avl, 1,
                                                   &avl);
    test_ne(node, NULL);

    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_eq(node->parent, NULL);
    test_eq_short(node->height, 0);
    test_eq_size(node->type, 1UL);
    test_eq(node->value, &avl);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(node_clean)
{
    lexbor_avl_t avl;
    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_node_t *node = lexbor_avl_node_make(&avl, 1,
                                                   &avl);
    test_ne(node, NULL);

    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_eq(node->parent, NULL);
    test_eq_short(node->height, 0);
    test_eq_size(node->type, 1UL);
    test_eq(node->value, &avl);

    lexbor_avl_node_clean(node);

    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_eq(node->parent, NULL);
    test_eq_short(node->height, 0);
    test_eq_size(node->type, 0UL);
    test_eq(node->value, NULL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(node_destroy)
{
    lexbor_avl_t *avl = lexbor_avl_create();
    lexbor_avl_init(avl, 1024);

    lexbor_avl_node_t *node = lexbor_avl_node_make(avl, 1,
                                                   &avl);
    test_ne(node, NULL);

    test_eq(lexbor_avl_node_destroy(avl, node, true), NULL);

    node = lexbor_avl_node_make(avl, 1, &avl);
    test_ne(node, NULL);

    test_eq(lexbor_avl_node_destroy(avl, node, false), node);
    test_eq(lexbor_avl_node_destroy(avl, NULL, false), NULL);

    lexbor_avl_destroy(avl, true);
}
TEST_END

TEST_BEGIN_ARGS(test_for_three, lexbor_avl_t *avl,
                lexbor_avl_node_t *root)
{
    lexbor_avl_node_t *node;

    test_ne(root, NULL);
    test_eq_size(root->type, 2UL);

    /* 1 */
    node = lexbor_avl_search(avl, root, 1);
    test_ne(node, NULL);

    test_eq_size(node->type, 1UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    /* 2 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 2UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 1UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 3UL);

    test_eq(node->parent, NULL);

    /* 3 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);
}
TEST_END

TEST_BEGIN(tree_3_0)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);

    TEST_CALL_ARGS(test_for_three, &avl, root);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(tree_3_1)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 1, (void *) 1);

    TEST_CALL_ARGS(test_for_three, &avl, root);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(tree_3_2)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);

    TEST_CALL_ARGS(test_for_three, &avl, root);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(tree_3_3)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 1, (void *) 1);

    TEST_CALL_ARGS(test_for_three, &avl, root);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(tree_3_4)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);

    TEST_CALL_ARGS(test_for_three, &avl, root);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(tree_3_5)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);

    TEST_CALL_ARGS(test_for_three, &avl, root);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(tree_4)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);

    /* 1 */
    node = lexbor_avl_search(&avl, root, 1);
    test_ne(node, NULL);

    test_eq_size(node->type, 1UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    /* 2 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 2UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 1UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 3UL);

    test_eq(node->parent, NULL);

    /* 3 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);
    test_eq(node->left, NULL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 4UL);

    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    /* 4 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 3UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(tree_5)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 5, (void *) 5);

    /* 1 */
    node = lexbor_avl_search(&avl, root, 1);
    test_ne(node, NULL);

    test_eq_size(node->type, 1UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    /* 2 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 2UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 1UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 4UL);

    test_eq(node->parent, NULL);

    /* 4 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 3UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 5UL);

    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    /* 3 */
    node = node->left;
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 4UL);

    /* 5 */
    node = node->parent->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 5UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 4UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_1L)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 1), NULL);
    test_ne(root, NULL);

    /* 2 */
    node = lexbor_avl_search(&avl, root, 2);
    test_ne(node, NULL);

    test_eq_size(node->type, 2UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 3UL);

    /* 3 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 2UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 4UL);

    test_eq(node->parent, NULL);

    /* 4 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 3UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_1R)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 4), NULL);
    test_ne(root, NULL);

    /* 1 */
    node = lexbor_avl_search(&avl, root, 1);
    test_ne(node, NULL);

    test_eq_size(node->type, 1UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    /* 2 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 2UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 1UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 3UL);

    test_eq(node->parent, NULL);

    /* 3 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_2L)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 1), NULL);
    test_ne(root, NULL);

    /* 2 */
    node = lexbor_avl_search(&avl, root, 2);
    test_ne(node, NULL);

    test_eq_size(node->type, 2UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 3UL);

    /* 3 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 2UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 4UL);

    test_eq(node->parent, NULL);

    /* 4 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 3UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_2R)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 4), NULL);
    test_ne(root, NULL);

    /* 1 */
    node = lexbor_avl_search(&avl, root, 1);
    test_ne(node, NULL);

    test_eq_size(node->type, 1UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    /* 2 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 2UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 1UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 3UL);

    test_eq(node->parent, NULL);

    /* 3 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_sub_1L)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 5, (void *) 5);
    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 6, (void *) 6);
    lexbor_avl_insert(&avl, &root, 7, (void *) 7);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 1), NULL);
    test_ne(root, NULL);

    /* 2 */
    node = lexbor_avl_search(&avl, root, 2);
    test_ne(node, NULL);

    test_eq_size(node->type, 2UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 3UL);

    /* 3 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 2UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 4UL);

    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 5UL);

    /* 4 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 3UL);

    /* 5 */
    node = node->parent->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 5UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 3UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 6UL);

    test_eq(node->parent, NULL);

    /* 6 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 6UL);
    test_eq(node->left, NULL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 7UL);

    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 5UL);

    /* 7 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 7UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 6UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_sub_1R)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 5, (void *) 5);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 6, (void *) 6);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 7, (void *) 7);
    lexbor_avl_insert(&avl, &root, 1, (void *) 1);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 7), NULL);
    test_ne(root, NULL);

    /* 1 */
    node = lexbor_avl_search(&avl, root, 1);
    test_ne(node, NULL);

    test_eq_size(node->type, 1UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 2UL);

    /* 2 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 2UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 1UL);

    test_eq(node->right, NULL);

    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 3UL);

    /* 3 */
    node = node->parent;
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 2UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 5UL);

    test_eq(node->parent, NULL);

    /* 5 */
    node = node->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 5UL);

    test_ne(node->left, NULL);
    test_eq_size(node->left->type, 4UL);

    test_ne(node->right, NULL);
    test_eq_size(node->right->type, 6UL);

    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 3UL);

    /* 4 */
    node = node->left;
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 5UL);

    /* 6 */
    node = node->parent->right;
    test_ne(node, NULL);

    test_eq_size(node->type, 6UL);
    test_eq(node->left, NULL);
    test_eq(node->right, NULL);
    test_ne(node->parent, NULL);
    test_eq_size(node->parent->type, 5UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_10_0)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 5, (void *) 5);
    lexbor_avl_insert(&avl, &root, 6, (void *) 6);
    lexbor_avl_insert(&avl, &root, 7, (void *) 7);
    lexbor_avl_insert(&avl, &root, 8, (void *) 8);
    lexbor_avl_insert(&avl, &root, 9, (void *) 9);
    lexbor_avl_insert(&avl, &root, 10, (void *) 10);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 8), NULL);
    test_ne(root, NULL);

    /* 4 */
    node = lexbor_avl_search(&avl, root, 4);
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);

    test_ne(node->left, NULL);
    test_ne(node->right, NULL);
    test_eq(node->parent, NULL);

    test_eq_size(node->left->type, 2UL);
    test_eq_size(node->left->left->type, 1UL);
    test_eq_size(node->left->right->type, 3UL);

    test_eq_size(node->right->type, 7UL);
    test_eq_size(node->right->left->type, 6UL);
    test_eq_size(node->right->right->type, 9UL);
    test_eq_size(node->right->left->left->type, 5UL);
    test_eq_size(node->right->right->right->type, 10UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_10_1)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 5, (void *) 5);
    lexbor_avl_insert(&avl, &root, 6, (void *) 6);
    lexbor_avl_insert(&avl, &root, 7, (void *) 7);
    lexbor_avl_insert(&avl, &root, 8, (void *) 8);
    lexbor_avl_insert(&avl, &root, 9, (void *) 9);
    lexbor_avl_insert(&avl, &root, 10, (void *) 10);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 8), NULL);
    test_ne(root, NULL);
    test_ne(lexbor_avl_remove(&avl, &root, 5), NULL);
    test_ne(root, NULL);

    /* 4 */
    node = lexbor_avl_search(&avl, root, 4);
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);

    test_ne(node->left, NULL);
    test_ne(node->right, NULL);
    test_eq(node->parent, NULL);

    test_eq_size(node->left->type, 2UL);
    test_eq_size(node->left->left->type, 1UL);
    test_eq_size(node->left->right->type, 3UL);

    test_eq_size(node->right->type, 7UL);
    test_eq_size(node->right->left->type, 6UL);
    test_eq_size(node->right->right->type, 9UL);
    test_eq_size(node->right->right->right->type, 10UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_10_2)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 5, (void *) 5);
    lexbor_avl_insert(&avl, &root, 6, (void *) 6);
    lexbor_avl_insert(&avl, &root, 7, (void *) 7);
    lexbor_avl_insert(&avl, &root, 8, (void *) 8);
    lexbor_avl_insert(&avl, &root, 9, (void *) 9);
    lexbor_avl_insert(&avl, &root, 10, (void *) 10);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 8), NULL);
    test_ne(root, NULL);
    test_ne(lexbor_avl_remove(&avl, &root, 6), NULL);
    test_ne(root, NULL);

    /* 4 */
    node = lexbor_avl_search(&avl, root, 4);
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);

    test_ne(node->left, NULL);
    test_ne(node->right, NULL);
    test_eq(node->parent, NULL);

    test_eq_size(node->left->type, 2UL);
    test_eq_size(node->left->left->type, 1UL);
    test_eq_size(node->left->right->type, 3UL);

    test_eq_size(node->right->type, 7UL);
    test_eq_size(node->right->left->type, 5UL);
    test_eq_size(node->right->right->type, 9UL);
    test_eq_size(node->right->right->right->type, 10UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_10_3)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 5, (void *) 5);
    lexbor_avl_insert(&avl, &root, 6, (void *) 6);
    lexbor_avl_insert(&avl, &root, 7, (void *) 7);
    lexbor_avl_insert(&avl, &root, 8, (void *) 8);
    lexbor_avl_insert(&avl, &root, 9, (void *) 9);
    lexbor_avl_insert(&avl, &root, 10, (void *) 10);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 9), NULL);
    test_ne(root, NULL);

    /* 4 */
    node = lexbor_avl_search(&avl, root, 4);
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);

    test_ne(node->left, NULL);
    test_ne(node->right, NULL);
    test_eq(node->parent, NULL);

    test_eq_size(node->left->type, 2UL);
    test_eq_size(node->left->left->type, 1UL);
    test_eq_size(node->left->right->type, 3UL);

    test_eq_size(node->right->type, 8UL);
    test_eq_size(node->right->left->type, 6UL);
    test_eq_size(node->right->right->type, 10UL);
    test_eq_size(node->right->left->left->type, 5UL);
    test_eq_size(node->right->left->right->type, 7UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_10_4)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 5, (void *) 5);
    lexbor_avl_insert(&avl, &root, 6, (void *) 6);
    lexbor_avl_insert(&avl, &root, 7, (void *) 7);
    lexbor_avl_insert(&avl, &root, 8, (void *) 8);
    lexbor_avl_insert(&avl, &root, 9, (void *) 9);
    lexbor_avl_insert(&avl, &root, 10, (void *) 10);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 4), NULL);
    test_ne(root, NULL);

    /* 3 */
    node = lexbor_avl_search(&avl, root, 3);
    test_ne(node, NULL);

    test_eq_size(node->type, 3UL);

    test_ne(node->left, NULL);
    test_ne(node->right, NULL);
    test_eq(node->parent, NULL);

    test_eq_size(node->left->type, 2UL);
    test_eq_size(node->left->left->type, 1UL);

    test_eq_size(node->right->type, 8UL);
    test_eq_size(node->right->left->type, 6UL);
    test_eq_size(node->right->right->type, 9UL);
    test_eq_size(node->right->left->left->type, 5UL);
    test_eq_size(node->right->left->right->type, 7UL);
    test_eq_size(node->right->right->right->type, 10UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(delete_10_5)
{
    lexbor_avl_t avl;
    lexbor_avl_node_t *root = NULL, *node;

    test_eq(lexbor_avl_init(&avl, 1024), LXB_STATUS_OK);

    lexbor_avl_insert(&avl, &root, 1, (void *) 1);
    lexbor_avl_insert(&avl, &root, 2, (void *) 2);
    lexbor_avl_insert(&avl, &root, 3, (void *) 3);
    lexbor_avl_insert(&avl, &root, 4, (void *) 4);
    lexbor_avl_insert(&avl, &root, 5, (void *) 5);
    lexbor_avl_insert(&avl, &root, 6, (void *) 6);
    lexbor_avl_insert(&avl, &root, 7, (void *) 7);
    lexbor_avl_insert(&avl, &root, 8, (void *) 8);
    lexbor_avl_insert(&avl, &root, 9, (void *) 9);
    lexbor_avl_insert(&avl, &root, 10, (void *) 10);

    test_ne(root, NULL);

    test_ne(lexbor_avl_remove(&avl, &root, 6), NULL);
    test_ne(root, NULL);

    /* 4 */
    node = lexbor_avl_search(&avl, root, 4);
    test_ne(node, NULL);

    test_eq_size(node->type, 4UL);

    test_ne(node->left, NULL);
    test_ne(node->right, NULL);
    test_eq(node->parent, NULL);

    test_eq_size(node->left->type, 2UL);
    test_eq_size(node->left->left->type, 1UL);
    test_eq_size(node->left->right->type, 3UL);

    test_eq_size(node->right->type, 8UL);
    test_eq_size(node->right->left->type, 5UL);
    test_eq_size(node->right->right->type, 9UL);
    test_eq_size(node->right->left->right->type, 7UL);
    test_eq_size(node->right->right->right->type, 10UL);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_avl_t avl;
    lexbor_avl_init(&avl, 1024);

    lexbor_avl_clean(&avl);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_avl_t *avl = lexbor_avl_create();
    lexbor_avl_init(avl, 1024);

    test_eq(lexbor_avl_destroy(avl, true), NULL);

    avl = lexbor_avl_create();
    lexbor_avl_init(avl, 1021);

    test_eq(lexbor_avl_destroy(avl, false), avl);
    test_eq(lexbor_avl_destroy(avl, true), NULL);
    test_eq(lexbor_avl_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_avl_t avl;
    lexbor_avl_init(&avl, 1023);

    test_eq(lexbor_avl_destroy(&avl, false), &avl);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(init);
    TEST_ADD(init_null);
    TEST_ADD(init_stack);
    TEST_ADD(init_args);
    TEST_ADD(node_make);
    TEST_ADD(node_clean);
    TEST_ADD(node_destroy);
    TEST_ADD(tree_3_0);
    TEST_ADD(tree_3_1);
    TEST_ADD(tree_3_2);
    TEST_ADD(tree_3_3);
    TEST_ADD(tree_3_4);
    TEST_ADD(tree_3_5);
    TEST_ADD(tree_4);
    TEST_ADD(tree_5);
    TEST_ADD(delete_1L);
    TEST_ADD(delete_1R);
    TEST_ADD(delete_2L);
    TEST_ADD(delete_2R);
    TEST_ADD(delete_sub_1L);
    TEST_ADD(delete_sub_1R);
    TEST_ADD(delete_10_0);
    TEST_ADD(delete_10_1);
    TEST_ADD(delete_10_2);
    TEST_ADD(delete_10_3);
    TEST_ADD(delete_10_4);
    TEST_ADD(delete_10_5);
    TEST_ADD(clean);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/avl");
    TEST_RELEASE();
}
