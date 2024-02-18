/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/bst.h>


TEST_BEGIN(init)
{
    lexbor_bst_t *bst = lexbor_bst_create();
    lxb_status_t status = lexbor_bst_init(bst, 128);

    test_eq(status, LXB_STATUS_OK);

    lexbor_bst_destroy(bst, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_bst_init(NULL, 128);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_bst_t bst;
    lxb_status_t status = lexbor_bst_init(&bst, 128);

    test_eq(status, LXB_STATUS_OK);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_bst_t bst = {0};
    lxb_status_t status;

    status = lexbor_bst_init(&bst, 0);
    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN_ARGS(test_for_push, lexbor_bst_t *bst)
{
    lexbor_bst_entry_t *entry, *pushed;

    test_ne(lexbor_bst_insert(bst, &bst->root, 5, (void *) 5), NULL);
    test_ne(lexbor_bst_insert(bst, &bst->root, 2, (void *) 2), NULL);
    test_ne(lexbor_bst_insert(bst, &bst->root, 1, (void *) 1), NULL);
    test_ne(lexbor_bst_insert(bst, &bst->root, 3, (void *) 3), NULL);
    test_ne(lexbor_bst_insert(bst, &bst->root, 18, (void *) 18), NULL);

    pushed = lexbor_bst_insert(bst, &bst->root, 4, (void *) 4);
    test_ne(pushed, NULL);

    entry = bst->root->left->right->right;
    test_eq(entry, pushed);
}
TEST_END

TEST_BEGIN(bst_insert)
{
    lexbor_bst_t bst;
    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    TEST_CALL_ARGS(test_for_push, &bst);

    test_eq_size(bst.root->size, 5UL);
    test_eq_size(bst.root->left->size, 2UL);
    test_eq_size(bst.root->left->left->size, 1UL);
    test_eq_size(bst.root->left->right->size, 3UL);
    test_eq_size(bst.root->left->right->right->size, 4UL);
    test_eq_size(bst.root->right->size, 18UL);

    test_eq_size(bst.tree_length, 6UL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(bst_search)
{
    lexbor_bst_t bst;
    lexbor_bst_entry_t *entry;

    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    TEST_CALL_ARGS(test_for_push, &bst);

    entry = lexbor_bst_search(&bst, lexbor_bst_root(&bst), 3);
    test_ne(entry, NULL);
    test_eq_size(entry->size, 3UL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(bst_search_close)
{
    lexbor_bst_t bst;
    lexbor_bst_entry_t *entry;

    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    TEST_CALL_ARGS(test_for_push, &bst);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 6);
    test_ne(entry, NULL);
    test_eq_size(entry->size, 18UL);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 0);
    test_ne(entry, NULL);
    test_eq_size(entry->size, 1UL);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 19);
    test_eq(entry, NULL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(bst_search_close_more)
{
    lexbor_bst_t bst;
    lexbor_bst_entry_t *entry;

    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    test_ne(lexbor_bst_insert(&bst, &bst.root, 76, (void *) 76), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 5, (void *) 5), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 2, (void *) 2), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 3, (void *) 3), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 36, (void *) 36), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 30, (void *) 30), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 8, (void *) 8), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 18, (void *) 18), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 21, (void *) 21), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 33, (void *) 33), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 31, (void *) 31), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 58, (void *) 58), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 63, (void *) 63), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 77, (void *) 77), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 84, (void *) 84), NULL);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 1);
    test_ne(entry, NULL);
    test_eq_size(entry->size, 2UL);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 29);
    test_ne(entry, NULL);
    test_eq_size(entry->size, 30UL);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 9);
    test_ne(entry, NULL);
    test_eq_size(entry->size, 18UL);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 32);
    test_ne(entry, NULL);
    test_eq_size(entry->size, 33UL);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 50);
    test_ne(entry, NULL);
    test_eq_size(entry->size, 58UL);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 80);
    test_ne(entry, NULL);
    test_eq_size(entry->size, 84UL);

    entry = lexbor_bst_search_close(&bst, lexbor_bst_root(&bst), 100);
    test_eq(entry, NULL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(bst_remove)
{
    lexbor_bst_t bst;
    void *value;
    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    TEST_CALL_ARGS(test_for_push, &bst);

    value = lexbor_bst_remove(&bst, &bst.root, 1);
    test_ne(value, NULL);

    test_eq_size(bst.root->size, 5UL);
    test_eq_size(bst.root->left->size, 2UL);
    test_eq_size(bst.root->left->right->size, 3UL);
    test_eq_size(bst.root->left->right->right->size, 4UL);
    test_eq_size(bst.root->right->size, 18UL);

    test_eq(bst.root->left->left, NULL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(bst_remove_one_child)
{
    lexbor_bst_t bst;
    void *value;
    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    TEST_CALL_ARGS(test_for_push, &bst);

    value = lexbor_bst_remove(&bst, &bst.root, 3);
    test_ne(value, NULL);

    test_eq_size(bst.root->size, 5UL);
    test_eq_size(bst.root->left->size, 2UL);
    test_eq_size(bst.root->left->left->size, 1UL);
    test_eq_size(bst.root->left->right->size, 4UL);
    test_eq_size(bst.root->right->size, 18UL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(bst_remove_two_child)
{
    lexbor_bst_t bst;
    void *value;
    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    test_ne(lexbor_bst_insert(&bst, &bst.root, 5, (void *) 5), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 2, (void *) 2), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 1, (void *) 1), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 3, (void *) 3), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 4, (void *) 4), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 12, (void *) 12), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 9, (void *) 9), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 21, (void *) 21), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 19, (void *) 19), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 25, (void *) 25), NULL);

    test_eq_size(bst.root->size, 5UL);
    test_eq_size(bst.root->left->size, 2UL);
    test_eq_size(bst.root->left->left->size, 1UL);
    test_eq_size(bst.root->left->right->size, 3UL);
    test_eq_size(bst.root->left->right->right->size, 4UL);
    test_eq_size(bst.root->right->size, 12UL);
    test_eq_size(bst.root->right->left->size, 9UL);
    test_eq_size(bst.root->right->right->size, 21UL);
    test_eq_size(bst.root->right->right->left->size, 19UL);
    test_eq_size(bst.root->right->right->right->size, 25UL);

    value = lexbor_bst_remove(&bst, &bst.root, 12);
    test_ne(value, NULL);

    test_eq_size(bst.root->size, 5UL);
    test_eq_size(bst.root->left->size, 2UL);
    test_eq_size(bst.root->left->left->size, 1UL);
    test_eq_size(bst.root->left->right->size, 3UL);
    test_eq_size(bst.root->left->right->right->size, 4UL);
    test_eq_size(bst.root->right->size, 19UL);
    test_eq_size(bst.root->right->left->size, 9UL);
    test_eq_size(bst.root->right->right->size, 21UL);
    test_eq_size(bst.root->right->right->right->size, 25UL);

    test_eq(bst.root->right->right->left, NULL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(bst_remove_root_two_child)
{
    lexbor_bst_t bst;
    void *value;
    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    test_ne(lexbor_bst_insert(&bst, &bst.root, 20, (void *) 20), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 10, (void *) 10), NULL);
    test_ne(lexbor_bst_insert(&bst, &bst.root, 5, (void *) 5), NULL);

    test_eq_size(bst.root->size, 20UL);
    test_eq(bst.root->parent, NULL);

    value = lexbor_bst_remove(&bst, &bst.root, 20);
    test_ne(value, NULL);

    test_eq_size(bst.root->size, 10UL);
    test_eq(bst.root->parent, NULL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(bst_remove_close)
{
    lexbor_bst_t bst;
    void *value;
    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    TEST_CALL_ARGS(test_for_push, &bst);

    value = lexbor_bst_remove_close(&bst, &bst.root, 7, NULL);
    test_ne(value, NULL);

    test_eq_size(bst.root->size, 5UL);
    test_eq_size(bst.root->left->size, 2UL);
    test_eq_size(bst.root->left->left->size, 1UL);
    test_eq_size(bst.root->left->right->size, 3UL);
    test_eq_size(bst.root->left->right->right->size, 4UL);

    test_eq(bst.root->right, NULL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_bst_t bst;
    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    lexbor_bst_entry_t *entry = lexbor_bst_insert(&bst, &bst.root, 100, NULL);
    test_ne(entry, NULL);
    test_eq_size(bst.tree_length, 1UL);

    lexbor_bst_clean(&bst);
    test_eq_size(bst.tree_length, 0UL);

    lexbor_bst_destroy(&bst, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_bst_t *bst = lexbor_bst_create();
    test_eq(lexbor_bst_init(bst, 128), LXB_STATUS_OK);

    test_eq(lexbor_bst_destroy(bst, true), NULL);

    bst = lexbor_bst_create();
    test_eq(lexbor_bst_init(bst, 128), LXB_STATUS_OK);

    test_eq(lexbor_bst_destroy(bst, false), bst);
    test_eq(lexbor_bst_destroy(bst, true), NULL);
    test_eq(lexbor_bst_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_bst_t bst;
    test_eq(lexbor_bst_init(&bst, 128), LXB_STATUS_OK);

    test_eq(lexbor_bst_destroy(&bst, false), &bst);
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
    TEST_ADD(clean);
    TEST_ADD(bst_insert);
    TEST_ADD(bst_search);
    TEST_ADD(bst_search_close);
    TEST_ADD(bst_search_close_more);
    TEST_ADD(bst_remove);
    TEST_ADD(bst_remove_one_child);
    TEST_ADD(bst_remove_two_child);
    TEST_ADD(bst_remove_root_two_child);
    TEST_ADD(bst_remove_close);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/bst");
    TEST_RELEASE();
}
