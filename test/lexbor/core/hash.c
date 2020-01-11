/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/hash.h>


typedef struct {
    lexbor_hash_t hash;
    size_t        value;
}
hash_entry_t;


TEST_BEGIN(init)
{
    lexbor_hash_t *hash = lexbor_hash_create();
    lxb_status_t status = lexbor_hash_init(hash, 1024, sizeof(hash_entry_t));
    test_eq(status, LXB_STATUS_OK);

    lexbor_hash_destroy(hash, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_hash_init(NULL, 1024, sizeof(hash_entry_t));
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_hash_t hash = {0};
    lxb_status_t status = lexbor_hash_init(&hash, 1024, sizeof(hash_entry_t));
    test_eq(status, LXB_STATUS_OK);

    lexbor_hash_destroy(&hash, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_hash_t hash;
    lxb_status_t status = lexbor_hash_init(&hash, 1024, sizeof(hash_entry_t));
    test_eq(status, LXB_STATUS_OK);

    lexbor_hash_clean(&hash);

    lexbor_hash_destroy(&hash, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_hash_t *hash = lexbor_hash_create();
    lxb_status_t status = lexbor_hash_init(hash, 1024, sizeof(hash_entry_t));
    test_eq(status, LXB_STATUS_OK);

    test_eq(lexbor_hash_destroy(hash, true), NULL);

    hash = lexbor_hash_create();
    status = lexbor_hash_init(hash, 1021, sizeof(hash_entry_t));
    test_eq(status, LXB_STATUS_OK);

    test_eq(lexbor_hash_destroy(hash, false), hash);
    test_eq(lexbor_hash_destroy(hash, true), NULL);
    test_eq(lexbor_hash_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_hash_t hash;
    lxb_status_t status = lexbor_hash_init(&hash, 1023, sizeof(hash_entry_t));
    test_eq(status, LXB_STATUS_OK);

    test_eq(lexbor_hash_destroy(&hash, false), &hash);
}
TEST_END

TEST_BEGIN(insert_search_case)
{
    lexbor_hash_t hash = {0};
    lxb_status_t status = lexbor_hash_init(&hash, 1024, sizeof(hash_entry_t));
    test_eq(status, LXB_STATUS_OK);

    hash_entry_t *entry, *entry_raw, *entry_lo, *entry_up;

    /* Raw */
    entry_raw = lexbor_hash_insert(&hash, lexbor_hash_insert_raw,
                                   (lxb_char_t *) "KeY", 3);
    test_ne(entry_raw, NULL);

    entry_raw->value = 1;

    /* Lower */
    entry_lo = lexbor_hash_insert(&hash, lexbor_hash_insert_lower,
                                  (lxb_char_t *) "KeY", 3);
    test_ne(entry_lo, NULL);

    entry_raw->value = 2;

    /* Upper */
    entry_up = lexbor_hash_insert(&hash, lexbor_hash_insert_upper,
                                  (lxb_char_t *) "kEy", 3);
    test_ne(entry_up, NULL);

    entry_up->value = 3;

    /* Check */
    /* Raw */
    entry = lexbor_hash_search(&hash, lexbor_hash_search_raw,
                               (lxb_char_t *) "KeY", 3);
    test_ne(entry, NULL);
    test_eq(entry, entry_raw);

    entry = lexbor_hash_search(&hash, lexbor_hash_search_raw,
                               (lxb_char_t *) "key", 3);
    test_ne(entry, NULL);
    test_eq(entry, entry_lo);

    entry = lexbor_hash_search(&hash, lexbor_hash_search_raw,
                               (lxb_char_t *) "KEY", 3);
    test_ne(entry, NULL);
    test_eq(entry, entry_up);

    entry = lexbor_hash_search(&hash, lexbor_hash_search_raw,
                               (lxb_char_t *) "keY", 3);
    test_eq(entry, NULL);

    /* Lower */
    entry = lexbor_hash_search(&hash, lexbor_hash_search_lower,
                               (lxb_char_t *) "KeY", 3);
    test_ne(entry, NULL);
    test_eq(entry, entry_lo);

    /* Upper */
    entry = lexbor_hash_search(&hash, lexbor_hash_search_upper,
                               (lxb_char_t *) "kEy", 3);
    test_ne(entry, NULL);
    test_eq(entry, entry_up);

    test_eq(lexbor_hash_entries_count(&hash), 3);

    lexbor_hash_destroy(&hash, false);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(init);
    TEST_ADD(init_null);
    TEST_ADD(init_stack);
    TEST_ADD(clean);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);
    TEST_ADD(insert_search_case);

    TEST_RUN("lexbor/core/hash");
    TEST_RELEASE();
}
