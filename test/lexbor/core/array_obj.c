/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/array_obj.h>


typedef struct {
    char   *data;
    size_t len;
}
test_struct_t;


TEST_BEGIN(init)
{
    lexbor_array_obj_t *array = lexbor_array_obj_create();
    lxb_status_t status = lexbor_array_obj_init(array, 32, sizeof(test_struct_t));

    test_eq(status, LXB_STATUS_OK);

    lexbor_array_obj_destroy(array, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_array_obj_init(NULL, 32, sizeof(test_struct_t));
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lxb_status_t status;
    lexbor_array_obj_t array;

    status = lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));
    test_eq(status, LXB_STATUS_OK);

    lexbor_array_obj_destroy(&array, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_array_obj_t array;
    lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));

    lexbor_array_obj_push(&array);
    test_eq_size(lexbor_array_obj_length(&array), 1UL);

    lexbor_array_obj_clean(&array);
    test_eq_size(lexbor_array_obj_length(&array), 0UL);

    lexbor_array_obj_destroy(&array, false);
}
TEST_END

TEST_BEGIN(push)
{
    lexbor_array_obj_t array;
    lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));

    test_eq_size(lexbor_array_obj_length(&array), 0UL);

    test_struct_t *entry = lexbor_array_obj_push(&array);
    test_ne(entry, NULL);

    test_eq_size(lexbor_array_obj_length(&array), 1UL);
    test_eq(lexbor_array_obj_get(&array, 0), entry);

    lexbor_array_obj_destroy(&array, false);
}
TEST_END

TEST_BEGIN(pop)
{
    lexbor_array_obj_t array;
    lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));

    test_struct_t *entry = lexbor_array_obj_push(&array);
    test_ne(entry, NULL);

    test_eq(lexbor_array_obj_pop(&array), entry);
    test_eq_size(lexbor_array_obj_length(&array), 0UL);

    lexbor_array_obj_destroy(&array, false);
}
TEST_END

TEST_BEGIN(pop_if_empty)
{
    lexbor_array_obj_t array;
    lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));

    test_eq_size(lexbor_array_obj_length(&array), 0UL);
    test_eq(lexbor_array_obj_pop(&array), NULL);
    test_eq_size(lexbor_array_obj_length(&array), 0UL);

    lexbor_array_obj_destroy(&array, false);
}
TEST_END

TEST_BEGIN(get)
{
    lexbor_array_obj_t array;
    lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));

    test_eq(lexbor_array_obj_get(&array, 1), NULL);
    test_eq(lexbor_array_obj_get(&array, 0), NULL);

    test_struct_t *entry = lexbor_array_obj_push(&array);
    test_ne(entry, NULL);

    test_eq(lexbor_array_obj_get(&array, 0), entry);
    test_eq(lexbor_array_obj_get(&array, 1), NULL);
    test_eq(lexbor_array_obj_get(&array, 1000), NULL);

    lexbor_array_obj_destroy(&array, false);
}
TEST_END

TEST_BEGIN(delete)
{
    test_struct_t *entry;
    lexbor_array_obj_t array;

    lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));

    for (size_t i = 0; i < 10; i++) {
        entry = lexbor_array_obj_push(&array);
        entry->data = (char *) i;
        entry->len = i;
    }

    test_eq_size(lexbor_array_obj_length(&array), 10UL);

    lexbor_array_obj_delete(&array, 10, 100);
    test_eq_size(lexbor_array_obj_length(&array), 10UL);

    lexbor_array_obj_delete(&array, 100, 1);
    test_eq_size(lexbor_array_obj_length(&array), 10UL);

    lexbor_array_obj_delete(&array, 100, 0);
    test_eq_size(lexbor_array_obj_length(&array), 10UL);

    for (size_t i = 0; i < 10; i++) {
        entry = lexbor_array_obj_get(&array, i);
        test_eq(entry->data, (void *) i);
        test_eq_size(entry->len, i);
    }

    lexbor_array_obj_delete(&array, 4, 4);
    test_eq_size(lexbor_array_obj_length(&array), 6UL);

    lexbor_array_obj_delete(&array, 4, 0);
    test_eq_size(lexbor_array_obj_length(&array), 6UL);

    lexbor_array_obj_delete(&array, 0, 0);
    test_eq_size(lexbor_array_obj_length(&array), 6UL);

    entry = lexbor_array_obj_get(&array, 0);
    test_eq(entry->data, (void *) 0);
    test_eq_size(entry->len, 0UL);

    entry = lexbor_array_obj_get(&array, 1);
    test_eq(entry->data, (void *) 1);
    test_eq_size(entry->len, 1UL);

    entry = lexbor_array_obj_get(&array, 2);
    test_eq(entry->data, (void *) 2);
    test_eq_size(entry->len, 2UL);

    entry = lexbor_array_obj_get(&array, 3);
    test_eq(entry->data, (void *) 3);
    test_eq_size(entry->len, 3UL);

    entry = lexbor_array_obj_get(&array, 4);
    test_eq(entry->data, (void *) 8);
    test_eq_size(entry->len, 8UL);

    entry = lexbor_array_obj_get(&array, 5);
    test_eq(entry->data, (void *) 9);
    test_eq_size(entry->len, 9UL);

    lexbor_array_obj_delete(&array, 0, 1);
    test_eq_size(lexbor_array_obj_length(&array), 5UL);

    entry = lexbor_array_obj_get(&array, 0);
    test_eq(entry->data, (void *) 1);
    test_eq_size(entry->len, 1UL);

    entry = lexbor_array_obj_get(&array, 1);
    test_eq(entry->data, (void *) 2);
    test_eq_size(entry->len, 2UL);

    entry = lexbor_array_obj_get(&array, 2);
    test_eq(entry->data, (void *) 3);
    test_eq_size(entry->len, 3UL);

    entry = lexbor_array_obj_get(&array, 3);
    test_eq(entry->data, (void *) 8);
    test_eq_size(entry->len, 8UL);

    entry = lexbor_array_obj_get(&array, 4);
    test_eq(entry->data, (void *) 9);
    test_eq_size(entry->len, 9UL);

    lexbor_array_obj_delete(&array, 1, 1000);
    test_eq_size(lexbor_array_obj_length(&array), 1UL);

    entry = lexbor_array_obj_get(&array, 0);
    test_eq(entry->data, (void *) 1);
    test_eq_size(entry->len, 1UL);

    lexbor_array_obj_destroy(&array, false);
}
TEST_END

TEST_BEGIN(delete_if_empty)
{
    lexbor_array_obj_t array;
    lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));

    lexbor_array_obj_delete(&array, 0, 0);
    test_eq_size(lexbor_array_obj_length(&array), 0UL);

    lexbor_array_obj_delete(&array, 1, 0);
    test_eq_size(lexbor_array_obj_length(&array), 0UL);

    lexbor_array_obj_delete(&array, 1, 1);
    test_eq_size(lexbor_array_obj_length(&array), 0UL);

    lexbor_array_obj_delete(&array, 100, 1);
    test_eq_size(lexbor_array_obj_length(&array), 0UL);

    lexbor_array_obj_delete(&array, 10, 100);
    test_eq_size(lexbor_array_obj_length(&array), 0UL);

    lexbor_array_obj_destroy(&array, false);
}
TEST_END

TEST_BEGIN(expand)
{
    lexbor_array_obj_t array;
    lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));

    test_ne(lexbor_array_obj_expand(&array, 128), NULL);
    test_eq_size(lexbor_array_obj_size(&array), 128UL);

    lexbor_array_obj_destroy(&array, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_array_obj_t *array = lexbor_array_obj_create();
    lexbor_array_obj_init(array, 32, sizeof(test_struct_t));

    test_eq(lexbor_array_obj_destroy(array, true), NULL);

    array = lexbor_array_obj_create();
    lexbor_array_obj_init(array, 32, sizeof(test_struct_t));

    test_eq(lexbor_array_obj_destroy(array, false), array);
    test_eq(lexbor_array_obj_destroy(array, true), NULL);
    test_eq(lexbor_array_obj_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_array_obj_t array;
    lexbor_array_obj_init(&array, 32, sizeof(test_struct_t));

    test_eq(lexbor_array_obj_destroy(&array, false), &array);
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
    TEST_ADD(push);
    TEST_ADD(pop);
    TEST_ADD(pop_if_empty);
    TEST_ADD(delete);
    TEST_ADD(delete_if_empty);
    TEST_ADD(expand);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/array_obj");
    TEST_RELEASE();
}
