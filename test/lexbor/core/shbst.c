/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include <unit/test.h>

#include <lexbor/core/shbst.h>


TEST_BEGIN(init)
{
    lexbor_shbst_t *shbst = lexbor_shbst_create();
    lxb_status_t status = lexbor_shbst_init(shbst, 1024);

    test_eq(status, LXB_STATUS_OK);

    lexbor_shbst_destroy(shbst, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_shbst_init(NULL, 1024);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_shbst_t shbst;
    lxb_status_t status = lexbor_shbst_init(&shbst, 1024);

    test_eq(status, LXB_STATUS_OK);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_shbst_t shbst = {0};
    lxb_status_t status = lexbor_shbst_init(&shbst, 0);

    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(insert)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "USSR";
    size_t key_size = strlen((const char *) key);

    entry = lexbor_shbst_insert(&shbst, key, key_size, &shbst);
    test_ne(entry, NULL);

    test_ne(entry->key, NULL);
    test_ne(entry->key, key);
    test_eq(entry->value, &shbst);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(insert_key_null)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    entry = lexbor_shbst_insert(&shbst, NULL, 123, &shbst);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(insert_key_size_0)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "USSR";

    entry = lexbor_shbst_insert(&shbst, key, 0, &shbst);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(insert_key_null_0)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    entry = lexbor_shbst_insert(&shbst, NULL, 0, &shbst);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(insert_value_null)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "USSR";
    size_t key_size = strlen((const char *) key);

    entry = lexbor_shbst_insert(&shbst, key, key_size, NULL);
    test_ne(entry, NULL);

    test_ne(entry->key, NULL);
    test_ne(entry->key, key);
    test_eq(entry->value, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(search)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "Stalin";
    size_t key_size = strlen((const char *) key);

    entry = lexbor_shbst_insert(&shbst, key, key_size, &shbst);
    test_ne(entry, NULL);

    entry = lexbor_shbst_search(&shbst, key, key_size, false);
    test_ne(entry, NULL);

    test_ne(entry->key, NULL);
    test_ne(entry->key, key);
    test_eq(entry->value, &shbst);

    test_eq_u_str(key, entry->key);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(search_sensitive)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "Stalin";
    size_t key_size = strlen((const char *) key);

    const lxb_char_t *key_search = (lxb_char_t *) "stalin";
    size_t key_search_size = strlen((const char *) key_search);

    entry = lexbor_shbst_insert(&shbst, key, key_size, &shbst);
    test_ne(entry, NULL);

    entry = lexbor_shbst_search(&shbst, key_search, key_search_size, false);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(search_insensitive)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "Stalin";
    size_t key_size = strlen((const char *) key);

    const lxb_char_t *key_search = (lxb_char_t *) "stalin";
    size_t key_search_size = strlen((const char *) key_search);

    entry = lexbor_shbst_insert(&shbst, key, key_size, &shbst);
    test_ne(entry, NULL);

    entry = lexbor_shbst_search(&shbst, key_search, key_search_size, true);
    test_ne(entry, NULL);

    test_ne(entry->key, NULL);
    test_ne(entry->key, key);
    test_eq(entry->value, &shbst);

    test_eq_u_str(key, entry->key);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(search_key_null)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    entry = lexbor_shbst_search(&shbst, NULL, 123, false);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(search_key_size_0)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "Stalin";

    entry = lexbor_shbst_search(&shbst, key, 0, false);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(search_key_null_0)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    entry = lexbor_shbst_search(&shbst, NULL, 0, false);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(replace)
{
    const lxb_char_t *stored_key;
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry, *entry_rep;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "KGB";
    size_t key_size = strlen((const char *) key);

    /* Insert */
    entry = lexbor_shbst_insert(&shbst, key, key_size, &shbst);
    test_ne(entry, NULL);

    test_eq(entry->value, &shbst);
    stored_key = entry->key;

    /* Replace */
    entry_rep = lexbor_shbst_replace(&shbst, key, key_size,
                                     (void *) key, false);
    test_eq(entry_rep, entry);

    test_eq(entry_rep->key, stored_key);
    test_eq(entry_rep->value, key);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(replace_sensitive)
{
    const lxb_char_t *stored_key;
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry, *entry_rep;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "KGB";
    size_t key_size = strlen((const char *) key);

    const lxb_char_t *key_rep = (lxb_char_t *) "KgB";
    size_t key_rep_size = strlen((const char *) key_rep);

    /* Insert */
    entry = lexbor_shbst_insert(&shbst, key, key_size, &shbst);
    test_ne(entry, NULL);

    test_eq(entry->value, &shbst);
    stored_key = entry->key;

    test_eq_u_str(entry->key, key);

    /* Replace */
    entry_rep = lexbor_shbst_replace(&shbst, key_rep, key_rep_size,
                                     (void *) key, false);
    test_ne(entry_rep, entry);

    test_ne(entry_rep->key, stored_key);
    test_eq(entry_rep->value, key);

    test_eq_u_str(entry_rep->key, key_rep);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(replace_insensitive)
{
    const lxb_char_t *stored_key;
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry, *entry_rep;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "KGB";
    size_t key_size = strlen((const char *) key);

    const lxb_char_t *key_rep = (lxb_char_t *) "KgB";
    size_t key_rep_size = strlen((const char *) key_rep);

    /* Insert */
    entry = lexbor_shbst_insert(&shbst, key, key_size, &shbst);
    test_ne(entry, NULL);

    test_eq(entry->value, &shbst);
    stored_key = entry->key;

    test_eq_u_str(entry->key, key);

    /* Replace */
    entry_rep = lexbor_shbst_replace(&shbst, key_rep, key_rep_size,
                                     (void *) key, true);
    test_eq(entry_rep, entry);

    test_eq(entry_rep->key, stored_key);
    test_eq(entry_rep->value, key);

    test_eq_u_str(entry_rep->key, stored_key);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(replace_not_exists)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "KGB";
    size_t key_size = strlen((const char *) key);

    entry = lexbor_shbst_replace(&shbst, key, key_size, &shbst, true);
    test_ne(entry, NULL);

    test_ne(entry->key, NULL);
    test_ne(entry->key, key);
    test_eq(entry->value, &shbst);

    test_eq_u_str(entry->key, key);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(replace_key_null)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    entry = lexbor_shbst_replace(&shbst, NULL, 321, &shbst, true);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(replace_key_size_0)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    const lxb_char_t *key = (lxb_char_t *) "KGB";

    entry = lexbor_shbst_replace(&shbst, key, 0, &shbst, true);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(replace_key_null_0)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_entry_t *entry;

    test_eq(lexbor_shbst_init(&shbst, 1024), LXB_STATUS_OK);

    entry = lexbor_shbst_replace(&shbst, NULL, 0, &shbst, true);
    test_eq(entry, NULL);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_init(&shbst, 1024);

    lexbor_shbst_clean(&shbst);

    lexbor_shbst_destroy(&shbst, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_shbst_t *shbst = lexbor_shbst_create();
    lexbor_shbst_init(shbst, 1024);

    test_eq(lexbor_shbst_destroy(shbst, true), NULL);

    shbst = lexbor_shbst_create();
    lexbor_shbst_init(shbst, 1021);

    test_eq(lexbor_shbst_destroy(shbst, false), shbst);
    test_eq(lexbor_shbst_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_shbst_t shbst;
    lexbor_shbst_init(&shbst, 1023);

    test_eq(lexbor_shbst_destroy(&shbst, false), &shbst);
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
    TEST_ADD(insert);
    TEST_ADD(insert_key_null);
    TEST_ADD(insert_key_size_0);
    TEST_ADD(insert_key_null_0);
    TEST_ADD(insert_value_null);
    TEST_ADD(search);
    TEST_ADD(search_sensitive);
    TEST_ADD(search_insensitive);
    TEST_ADD(search_key_null);
    TEST_ADD(search_key_size_0);
    TEST_ADD(search_key_null_0);
    TEST_ADD(replace);
    TEST_ADD(replace_sensitive);
    TEST_ADD(replace_insensitive);
    TEST_ADD(replace_not_exists);
    TEST_ADD(replace_key_null);
    TEST_ADD(replace_key_size_0);
    TEST_ADD(replace_key_null_0);
    TEST_ADD(clean);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/shbst");
    TEST_RELEASE();
}
