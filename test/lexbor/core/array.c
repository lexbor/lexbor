/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/array.h>


TEST_BEGIN(init)
{
    lexbor_array_t *array = lexbor_array_create();
    lxb_status_t status = lexbor_array_init(array, 32);

    test_eq(status, LXB_STATUS_OK);

    lexbor_array_destroy(array, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_array_init(NULL, 32);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_array_t array;
    lxb_status_t status = lexbor_array_init(&array, 32);

    test_eq(status, LXB_STATUS_OK);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    lexbor_array_push(&array, (void *) 1);
    test_eq_size(lexbor_array_length(&array), 1UL);

    lexbor_array_clean(&array);
    test_eq_size(lexbor_array_length(&array), 0UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(push)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    test_eq_size(lexbor_array_length(&array), 0UL);

    lexbor_array_push(&array, (void *) 1);

    test_eq_size(lexbor_array_length(&array), 1UL);
    test_eq(lexbor_array_get(&array, 0), (void *) 1);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(push_null)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    lexbor_array_push(&array, NULL);

    test_eq_size(lexbor_array_length(&array), 1UL);
    test_eq(lexbor_array_get(&array, 0), NULL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(pop)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    lexbor_array_push(&array, (void *) 123);

    test_eq(lexbor_array_pop(&array), (void *) 123);
    test_eq_size(lexbor_array_length(&array), 0UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(pop_if_empty)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    test_eq_size(lexbor_array_length(&array), 0UL);
    test_eq(lexbor_array_pop(&array), NULL);
    test_eq_size(lexbor_array_length(&array), 0UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(get)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    test_eq(lexbor_array_get(&array, 1), NULL);
    test_eq(lexbor_array_get(&array, 0), NULL);

    lexbor_array_push(&array, (void *) 123);

    test_eq(lexbor_array_get(&array, 0), (void *) 123);
    test_eq(lexbor_array_get(&array, 1), NULL);
    test_eq(lexbor_array_get(&array, 1000), NULL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(set)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    lexbor_array_push(&array, (void *) 123);

    test_eq(lexbor_array_set(&array, 0, (void *) 456), LXB_STATUS_OK);
    test_eq(lexbor_array_get(&array, 0), (void *) 456);

    test_eq_size(lexbor_array_length(&array), 1UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(set_not_exists)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    test_eq(lexbor_array_set(&array, 10, (void *) 123), LXB_STATUS_OK);
    test_eq(lexbor_array_get(&array, 10), (void *) 123);

    for (size_t i = 0; i < 10; i++) {
        test_eq(lexbor_array_get(&array, i), NULL);
    }

    test_eq_size(lexbor_array_length(&array), 11UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(insert)
{
    lxb_status_t status;
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    status = lexbor_array_insert(&array, 0, (void *) 456);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lexbor_array_get(&array, 0), (void *) 456);

    test_eq_size(lexbor_array_length(&array), 1UL);
    test_eq_size(lexbor_array_size(&array), 32UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(insert_end)
{
    lxb_status_t status;
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    status = lexbor_array_insert(&array, 32, (void *) 457);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lexbor_array_get(&array, 32), (void *) 457);

    test_eq_size(lexbor_array_length(&array), 33UL);
    test_ne_size(lexbor_array_size(&array), 32UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(insert_overflow)
{
    lxb_status_t status;
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    status = lexbor_array_insert(&array, 33, (void *) 458);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lexbor_array_get(&array, 33), (void *) 458);

    test_eq_size(lexbor_array_length(&array), 34UL);
    test_ne_size(lexbor_array_size(&array), 32UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(insert_to)
{
    lxb_status_t status;
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    test_eq(lexbor_array_push(&array, (void *) 1), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 2), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 3), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 4), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 5), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 6), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 7), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 8), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 9), LXB_STATUS_OK);

    status = lexbor_array_insert(&array, 4, (void *) 459);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lexbor_array_get(&array, 0), (void *) 1);
    test_eq(lexbor_array_get(&array, 1), (void *) 2);
    test_eq(lexbor_array_get(&array, 2), (void *) 3);
    test_eq(lexbor_array_get(&array, 3), (void *) 4);
    test_eq(lexbor_array_get(&array, 4), (void *) 459);
    test_eq(lexbor_array_get(&array, 5), (void *) 5);
    test_eq(lexbor_array_get(&array, 6), (void *) 6);
    test_eq(lexbor_array_get(&array, 7), (void *) 7);
    test_eq(lexbor_array_get(&array, 8), (void *) 8);
    test_eq(lexbor_array_get(&array, 9), (void *) 9);

    test_eq_size(lexbor_array_length(&array), 10UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(insert_to_end)
{
    lxb_status_t status;
    lexbor_array_t array;
    lexbor_array_init(&array, 9);

    test_eq(lexbor_array_push(&array, (void *) 1), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 2), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 3), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 4), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 5), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 6), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 7), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 8), LXB_STATUS_OK);
    test_eq(lexbor_array_push(&array, (void *) 9), LXB_STATUS_OK);

    test_eq_size(lexbor_array_length(&array), 9UL);
    test_eq_size(lexbor_array_size(&array), 9UL);

    status = lexbor_array_insert(&array, 4, (void *) 459);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lexbor_array_get(&array, 0), (void *) 1);
    test_eq(lexbor_array_get(&array, 1), (void *) 2);
    test_eq(lexbor_array_get(&array, 2), (void *) 3);
    test_eq(lexbor_array_get(&array, 3), (void *) 4);
    test_eq(lexbor_array_get(&array, 4), (void *) 459);
    test_eq(lexbor_array_get(&array, 5), (void *) 5);
    test_eq(lexbor_array_get(&array, 6), (void *) 6);
    test_eq(lexbor_array_get(&array, 7), (void *) 7);
    test_eq(lexbor_array_get(&array, 8), (void *) 8);
    test_eq(lexbor_array_get(&array, 9), (void *) 9);

    test_eq_size(lexbor_array_length(&array), 10UL);
    test_ne_size(lexbor_array_size(&array), 9UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(delete)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    for (size_t i = 0; i < 10; i++) {
        lexbor_array_push(&array, (void *) i);
    }

    test_eq_size(lexbor_array_length(&array), 10UL);

    lexbor_array_delete(&array, 10, 100);
    test_eq_size(lexbor_array_length(&array), 10UL);

    lexbor_array_delete(&array, 100, 1);
    test_eq_size(lexbor_array_length(&array), 10UL);

    lexbor_array_delete(&array, 100, 0);
    test_eq_size(lexbor_array_length(&array), 10UL);

    for (size_t i = 0; i < 10; i++) {
        test_eq(lexbor_array_get(&array, i), (void *) i);
    }

    lexbor_array_delete(&array, 4, 4);
    test_eq_size(lexbor_array_length(&array), 6UL);

    lexbor_array_delete(&array, 4, 0);
    test_eq_size(lexbor_array_length(&array), 6UL);

    lexbor_array_delete(&array, 0, 0);
    test_eq_size(lexbor_array_length(&array), 6UL);

    test_eq(lexbor_array_get(&array, 0), (void *) 0);
    test_eq(lexbor_array_get(&array, 1), (void *) 1);
    test_eq(lexbor_array_get(&array, 2), (void *) 2);
    test_eq(lexbor_array_get(&array, 3), (void *) 3);
    test_eq(lexbor_array_get(&array, 4), (void *) 8);
    test_eq(lexbor_array_get(&array, 5), (void *) 9);

    lexbor_array_delete(&array, 0, 1);
    test_eq_size(lexbor_array_length(&array), 5UL);

    test_eq(lexbor_array_get(&array, 0), (void *) 1);
    test_eq(lexbor_array_get(&array, 1), (void *) 2);
    test_eq(lexbor_array_get(&array, 2), (void *) 3);
    test_eq(lexbor_array_get(&array, 3), (void *) 8);
    test_eq(lexbor_array_get(&array, 4), (void *) 9);

    lexbor_array_delete(&array, 1, 1000);
    test_eq_size(lexbor_array_length(&array), 1UL);

    test_eq(lexbor_array_get(&array, 0), (void *) 1);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(delete_if_empty)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    lexbor_array_delete(&array, 0, 0);
    test_eq_size(lexbor_array_length(&array), 0UL);

    lexbor_array_delete(&array, 1, 0);
    test_eq_size(lexbor_array_length(&array), 0UL);

    lexbor_array_delete(&array, 1, 1);
    test_eq_size(lexbor_array_length(&array), 0UL);

    lexbor_array_delete(&array, 100, 1);
    test_eq_size(lexbor_array_length(&array), 0UL);

    lexbor_array_delete(&array, 10, 100);
    test_eq_size(lexbor_array_length(&array), 0UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(expand)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    test_ne(lexbor_array_expand(&array, 128), NULL);
    test_eq_size(lexbor_array_size(&array), 128UL);

    lexbor_array_destroy(&array, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_array_t *array = lexbor_array_create();
    lexbor_array_init(array, 32);

    test_eq(lexbor_array_destroy(array, true), NULL);

    array = lexbor_array_create();
    lexbor_array_init(array, 32);

    test_eq(lexbor_array_destroy(array, false), array);
    test_eq(lexbor_array_destroy(array, true), NULL);
    test_eq(lexbor_array_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_array_t array;
    lexbor_array_init(&array, 32);

    test_eq(lexbor_array_destroy(&array, false), &array);
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
    TEST_ADD(push_null);
    TEST_ADD(pop);
    TEST_ADD(pop_if_empty);
    TEST_ADD(delete);
    TEST_ADD(delete_if_empty);
    TEST_ADD(set);
    TEST_ADD(set_not_exists);
    TEST_ADD(insert);
    TEST_ADD(insert_end);
    TEST_ADD(insert_overflow);
    TEST_ADD(insert_to);
    TEST_ADD(insert_to_end);
    TEST_ADD(expand);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/array");
    TEST_RELEASE();
}

