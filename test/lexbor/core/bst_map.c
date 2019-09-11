/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/bst_map.h>


TEST_BEGIN(init)
{
    lexbor_bst_map_t *bst_map = lexbor_bst_map_create();
    lxb_status_t status = lexbor_bst_map_init(bst_map, 128);

    test_eq(status, LXB_STATUS_OK);

    lexbor_bst_map_destroy(bst_map, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_bst_map_init(NULL, 128);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_bst_map_t bst_map;
    lxb_status_t status = lexbor_bst_map_init(&bst_map, 128);

    test_eq(status, LXB_STATUS_OK);

    lexbor_bst_map_destroy(&bst_map, false);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_bst_map_t bst_map = {0};
    lxb_status_t status;

    status = lexbor_bst_map_init(&bst_map, 0);
    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    lexbor_bst_map_destroy(&bst_map, false);
}
TEST_END

TEST_BEGIN(bst_map_insert)
{
    lexbor_bst_map_t bst_map;
    lexbor_bst_map_entry_t *entry;

    lexbor_bst_entry_t *scope = NULL;

    static const lxb_char_t key[] = "test";
    size_t key_len = sizeof(key) - 1;

    test_eq(lexbor_bst_map_init(&bst_map, 128), LXB_STATUS_OK);

    entry = lexbor_bst_map_insert(&bst_map, &scope,
                                  key, key_len, (void *) 1);
    test_ne(entry, NULL);
    test_ne(scope, NULL);

    test_eq_u_str(entry->str.data, key);
    test_eq_size(entry->str.length, key_len);
    test_eq(entry->value, (void *) 1);

    lexbor_bst_map_destroy(&bst_map, false);
}
TEST_END

TEST_BEGIN(bst_map_search)
{
    lexbor_bst_map_t bst_map;
    lexbor_bst_map_entry_t *entry;

    lexbor_bst_entry_t *scope = NULL;

    static const lxb_char_t key[] = "test";
    size_t key_len = sizeof(key) - 1;

    static const lxb_char_t col_key[] = "test1";
    size_t col_key_len = sizeof(col_key) - 1;

    test_eq(lexbor_bst_map_init(&bst_map, 128), LXB_STATUS_OK);

    entry = lexbor_bst_map_insert(&bst_map, &scope,
                                  key, key_len, (void *) 1);
    test_ne(entry, NULL);

    entry = lexbor_bst_map_insert(&bst_map, &scope,
                                  col_key, col_key_len, (void *) 2);
    test_ne(entry, NULL);

    entry = lexbor_bst_map_search(&bst_map, scope, key, key_len);
    test_ne(entry, NULL);

    test_eq_u_str(entry->str.data, key);
    test_eq_size(entry->str.length, key_len);
    test_eq(entry->value, (void *) 1);

    lexbor_bst_map_destroy(&bst_map, false);
}
TEST_END

TEST_BEGIN(bst_map_remove)
{
    void *value;
    lexbor_bst_map_t bst_map;
    lexbor_bst_map_entry_t *entry;

    lexbor_bst_entry_t *scope = NULL;

    static const lxb_char_t key[] = "test";
    size_t key_len = sizeof(key) - 1;

    static const lxb_char_t col_key[] = "test1";
    size_t col_key_len = sizeof(col_key) - 1;

    test_eq(lexbor_bst_map_init(&bst_map, 128), LXB_STATUS_OK);

    entry = lexbor_bst_map_insert(&bst_map, &scope,
                                  key, key_len, (void *) 1);
    test_ne(entry, NULL);

    entry = lexbor_bst_map_insert(&bst_map, &scope,
                                  col_key, col_key_len, (void *) 2);
    test_ne(entry, NULL);

    value = lexbor_bst_map_remove(&bst_map, &scope, key, key_len);

    test_eq(value, (void *) 1);
    test_ne(scope, NULL);

    lexbor_bst_map_destroy(&bst_map, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_bst_map_t bst_map;
    lexbor_bst_map_entry_t *entry;
    lexbor_bst_entry_t *scope = NULL;

    static const lxb_char_t key[] = "test";
    size_t key_len = sizeof(key) - 1;

    test_eq(lexbor_bst_map_init(&bst_map, 128), LXB_STATUS_OK);

    entry = lexbor_bst_map_insert(&bst_map, &scope,
                                  key, key_len, (void *) 1);
    test_ne(entry, NULL);

    lexbor_bst_map_clean(&bst_map);

    lexbor_bst_map_destroy(&bst_map, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_bst_map_t *bst_map = lexbor_bst_map_create();
    test_eq(lexbor_bst_map_init(bst_map, 128), LXB_STATUS_OK);

    test_eq(lexbor_bst_map_destroy(bst_map, true), NULL);

    bst_map = lexbor_bst_map_create();
    test_eq(lexbor_bst_map_init(bst_map, 128), LXB_STATUS_OK);

    test_eq(lexbor_bst_map_destroy(bst_map, false), bst_map);
    test_eq(lexbor_bst_map_destroy(bst_map, true), NULL);
    test_eq(lexbor_bst_map_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_bst_map_t bst_map;
    test_eq(lexbor_bst_map_init(&bst_map, 128), LXB_STATUS_OK);

    test_eq(lexbor_bst_map_destroy(&bst_map, false), &bst_map);
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
    TEST_ADD(bst_map_insert);
    TEST_ADD(bst_map_search);
    TEST_ADD(bst_map_remove);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/bst_map");
    TEST_RELEASE();
}
