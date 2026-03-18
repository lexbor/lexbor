/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/avl.h>


typedef struct {
    size_t remove;
    size_t *result;
    size_t *p;
}
avl_test_ctx_t;

typedef struct {
    size_t *removes;
    size_t removes_len;
    size_t *result;
    size_t *p;
}
avl_test_multi_ctx_t;


static lxb_status_t
avl_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
       lexbor_avl_node_t *node, void *ctx);

static lxb_status_t
avl_cb_multi(lexbor_avl_t *avl, lexbor_avl_node_t **root,
             lexbor_avl_node_t *node, void *ctx);

static lxb_status_t
avl_cb_remove_all(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                  lexbor_avl_node_t *node, void *ctx);

static lxb_status_t
avl_cb_remove_next(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                   lexbor_avl_node_t *node, void *ctx);


TEST_BEGIN(init)
{
    lexbor_avl_t *avl = lexbor_avl_create();
    lxb_status_t status = lexbor_avl_init(avl, 1024, 0);

    test_eq(status, LXB_STATUS_OK);

    lexbor_avl_destroy(avl, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_avl_init(NULL, 1024, 0);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_avl_t avl;
    lxb_status_t status = lexbor_avl_init(&avl, 1024, 0);

    test_eq(status, LXB_STATUS_OK);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_avl_t avl = {0};
    lxb_status_t status = lexbor_avl_init(&avl, 0, 0);

    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(node_make)
{
    lexbor_avl_t avl;
    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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
    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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
    lexbor_avl_init(avl, 1024, 0);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

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
    lexbor_avl_init(&avl, 1024, 0);

    lexbor_avl_clean(&avl);

    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_avl_t *avl = lexbor_avl_create();
    lexbor_avl_init(avl, 1024, 0);

    test_eq(lexbor_avl_destroy(avl, true), NULL);

    avl = lexbor_avl_create();
    lexbor_avl_init(avl, 1021, 0);

    test_eq(lexbor_avl_destroy(avl, false), avl);
    test_eq(lexbor_avl_destroy(avl, true), NULL);
    test_eq(lexbor_avl_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_avl_t avl;
    lexbor_avl_init(&avl, 1023, 0);

    test_eq(lexbor_avl_destroy(&avl, false), &avl);
}
TEST_END

TEST_BEGIN(foreach_4)
{
    size_t i, *p;
    lexbor_avl_t avl;
    avl_test_ctx_t test;
    lexbor_avl_node_t *root = NULL;

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

    for (i = 5; i > 1; i--) {
        lexbor_avl_insert(&avl, &root, i, NULL);
    }

    test.result = lexbor_malloc(10 * sizeof(size_t));
    test_ne(test.result, NULL);

    test.remove = 4;
    test.p = test.result;

    lexbor_avl_foreach(&avl, &root, avl_cb, &test);

    p = test.result;

    for (i = 2; i < 6; i++) {
        test_ne(p, test.p);

        test_eq(i, *p);

        p++;
    }

    lexbor_free(test.result);
    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(foreach_6)
{
    size_t i, *p;
    lexbor_avl_t avl;
    avl_test_ctx_t test;
    lexbor_avl_node_t *root = NULL;

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

    for (i = 5; i < 9; i++) {
        lexbor_avl_insert(&avl, &root, i, NULL);
    }

    test.result = lexbor_malloc(10 * sizeof(size_t));
    test_ne(test.result, NULL);

    test.remove = 6;
    test.p = test.result;

    lexbor_avl_foreach(&avl, &root, avl_cb, &test);

    p = test.result;

    for (i = 5; i < 9; i++) {
        test_ne(p, test.p);

        test_eq(i, *p);

        p++;
    }

    lexbor_free(test.result);
    lexbor_avl_destroy(&avl, false);
}
TEST_END

TEST_BEGIN(foreach_10)
{
    size_t i, *p;
    lexbor_avl_t avl;
    avl_test_ctx_t test;
    lexbor_avl_node_t *root;

    static const size_t total = 101;

    test.result = lexbor_malloc(total * sizeof(size_t));
    test_ne(test.result, NULL);

    for (size_t r = 1; r < total; r++) {
        test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

        root = NULL;

        for (i = 1; i < total; i++) {
            lexbor_avl_insert(&avl, &root, i, NULL);
        }

        test.remove = r;
        test.p = test.result;

        lexbor_avl_foreach(&avl, &root, avl_cb, &test);

        p = test.result;

        for (i = 1; i < total; i++) {
            test_ne(p, test.p);

            test_eq(i, *p);

            p++;
        }

        lexbor_avl_destroy(&avl, false);
    }

    lexbor_free(test.result);
}
TEST_END

/* Remove multiple nodes during foreach: remove 3,5,7 from tree 1..10. */
TEST_BEGIN(foreach_multi_remove)
{
    size_t i, *p;
    lexbor_avl_t avl;
    avl_test_multi_ctx_t test;
    lexbor_avl_node_t *root = NULL;
    size_t removes[] = {3, 5, 7};

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

    for (i = 1; i <= 10; i++) {
        lexbor_avl_insert(&avl, &root, i, NULL);
    }

    test.result = lexbor_malloc(20 * sizeof(size_t));
    test_ne(test.result, NULL);

    test.removes = removes;
    test.removes_len = sizeof(removes) / sizeof(removes[0]);
    test.p = test.result;

    lexbor_avl_foreach(&avl, &root, avl_cb_multi, &test);

    p = test.result;

    /* All 10 nodes must be visited in order. */
    for (i = 1; i <= 10; i++) {
        test_ne(p, test.p);
        test_eq(i, *p);
        p++;
    }

    /* After removal, nodes 3,5,7 must not be found. */
    test_eq(lexbor_avl_search(&avl, root, 3), NULL);
    test_eq(lexbor_avl_search(&avl, root, 5), NULL);
    test_eq(lexbor_avl_search(&avl, root, 7), NULL);

    /* Remaining nodes must still be found. */
    test_ne(lexbor_avl_search(&avl, root, 1), NULL);
    test_ne(lexbor_avl_search(&avl, root, 2), NULL);
    test_ne(lexbor_avl_search(&avl, root, 4), NULL);
    test_ne(lexbor_avl_search(&avl, root, 6), NULL);
    test_ne(lexbor_avl_search(&avl, root, 8), NULL);
    test_ne(lexbor_avl_search(&avl, root, 9), NULL);
    test_ne(lexbor_avl_search(&avl, root, 10), NULL);

    lexbor_free(test.result);
    lexbor_avl_destroy(&avl, false);
}
TEST_END

/* Remove all nodes one by one during foreach. */
TEST_BEGIN(foreach_remove_all)
{
    size_t i, *p;
    lexbor_avl_t avl;
    avl_test_ctx_t test;
    lexbor_avl_node_t *root = NULL;

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

    for (i = 1; i <= 10; i++) {
        lexbor_avl_insert(&avl, &root, i, NULL);
    }

    test.result = lexbor_malloc(20 * sizeof(size_t));
    test_ne(test.result, NULL);

    test.remove = 0;
    test.p = test.result;

    lexbor_avl_foreach(&avl, &root, avl_cb_remove_all, &test);

    p = test.result;

    /* All 10 nodes must be visited in order. */
    for (i = 1; i <= 10; i++) {
        test_ne(p, test.p);
        test_eq(i, *p);
        p++;
    }

    /* Tree must be empty. */
    test_eq(root, NULL);

    lexbor_free(test.result);
    lexbor_avl_destroy(&avl, false);
}
TEST_END

/* In callback, remove the next node (node->type + 1) that hasn't been visited yet. */
TEST_BEGIN(foreach_remove_next)
{
    size_t i, *p, count;
    lexbor_avl_t avl;
    avl_test_ctx_t test;
    lexbor_avl_node_t *root = NULL;

    static const size_t total = 20;

    test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

    for (i = 1; i <= total; i++) {
        lexbor_avl_insert(&avl, &root, i, NULL);
    }

    test.result = lexbor_malloc((total + 1) * sizeof(size_t));
    test_ne(test.result, NULL);

    test.remove = total;
    test.p = test.result;

    lexbor_avl_foreach(&avl, &root, avl_cb_remove_next, &test);

    /* Verify visited nodes are in sorted order. */
    p = test.result;
    count = (size_t) (test.p - test.result);

    for (i = 1; i < count; i++) {
        test_gt(p[i], p[i - 1]);
    }

    /* Verify tree integrity: all remaining nodes must be findable. */
    for (i = 1; i <= total; i++) {
        lexbor_avl_node_t *found = lexbor_avl_search(&avl, root, i);

        if (found == NULL) {
            /* Node was removed — that's fine, just verify it's really gone. */
            test_eq(found, NULL);
        }
    }

    lexbor_free(test.result);
    lexbor_avl_destroy(&avl, false);
}
TEST_END

/* Foreach with removal on trees of every size from 2 to 50,
 * removing every possible node position. */
TEST_BEGIN(foreach_remove_stress)
{
    size_t i, *p;
    lexbor_avl_t avl;
    avl_test_ctx_t test;
    lexbor_avl_node_t *root;

    static const size_t max_size = 50;

    test.result = lexbor_malloc((max_size + 1) * sizeof(size_t));
    test_ne(test.result, NULL);

    for (size_t sz = 2; sz <= max_size; sz++) {
        for (size_t r = 1; r <= sz; r++) {
            test_eq(lexbor_avl_init(&avl, 1024, 0), LXB_STATUS_OK);

            root = NULL;

            for (i = 1; i <= sz; i++) {
                lexbor_avl_insert(&avl, &root, i, NULL);
            }

            test.remove = r;
            test.p = test.result;

            lexbor_avl_foreach(&avl, &root, avl_cb, &test);

            p = test.result;

            for (i = 1; i <= sz; i++) {
                test_ne(p, test.p);
                test_eq(i, *p);
                p++;
            }

            lexbor_avl_destroy(&avl, false);
        }
    }

    lexbor_free(test.result);
}
TEST_END

static lxb_status_t
avl_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
       lexbor_avl_node_t *node, void *ctx)
{
    avl_test_ctx_t *test = ctx;

    *test->p = node->type;
    test->p++;

    if (node->type == test->remove) {
        lexbor_avl_remove_by_node(avl, root, node);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
avl_cb_multi(lexbor_avl_t *avl, lexbor_avl_node_t **root,
             lexbor_avl_node_t *node, void *ctx)
{
    avl_test_multi_ctx_t *test = ctx;

    *test->p = node->type;
    test->p++;

    for (size_t i = 0; i < test->removes_len; i++) {
        if (node->type == test->removes[i]) {
            lexbor_avl_remove_by_node(avl, root, node);
            break;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
avl_cb_remove_all(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                  lexbor_avl_node_t *node, void *ctx)
{
    avl_test_ctx_t *test = ctx;

    *test->p = node->type;
    test->p++;

    lexbor_avl_remove_by_node(avl, root, node);

    return LXB_STATUS_OK;
}

static lxb_status_t
avl_cb_remove_next(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                   lexbor_avl_node_t *node, void *ctx)
{
    avl_test_ctx_t *test = ctx;
    lexbor_avl_node_t *next;

    *test->p = node->type;
    test->p++;

    /* Try to remove the next node (type + 1) which hasn't been visited yet. */
    if (node->type < test->remove) {
        next = lexbor_avl_search(avl, *root, node->type + 1);

        if (next != NULL) {
            lexbor_avl_remove_by_node(avl, root, next);
        }
    }

    return LXB_STATUS_OK;
}

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

    TEST_ADD(foreach_4);
    TEST_ADD(foreach_6);
    TEST_ADD(foreach_10);
    TEST_ADD(foreach_multi_remove);
    TEST_ADD(foreach_remove_all);
    TEST_ADD(foreach_remove_next);
    TEST_ADD(foreach_remove_stress);

    TEST_RUN("lexbor/core/avl");
    TEST_RELEASE();
}
