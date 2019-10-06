/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/dobject.h>


typedef struct {
    size_t a;
    char   b;
    int    c;
}
test_data_t;


TEST_BEGIN(init)
{
    lexbor_dobject_t *dobj = lexbor_dobject_create();
    lxb_status_t status = lexbor_dobject_init(dobj, 128, sizeof(test_data_t));

    test_eq(status, LXB_STATUS_OK);

    lexbor_dobject_destroy(dobj, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_dobject_init(NULL, 128, sizeof(test_data_t));
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_dobject_t dobj;
    lxb_status_t status = lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    test_eq(status, LXB_STATUS_OK);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_dobject_t dobj = {0};
    lxb_status_t status;

    status = lexbor_dobject_init(&dobj, 0, sizeof(test_data_t));
    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    status = lexbor_dobject_init(&dobj, 128, 0);
    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    status = lexbor_dobject_init(&dobj, 0, 0);
    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(obj_alloc)
{
    lexbor_dobject_t dobj;
    lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    test_data_t *data = lexbor_dobject_alloc(&dobj);

    test_ne(data, NULL);
    test_eq_size(lexbor_dobject_allocated(&dobj), 1UL);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(obj_calloc)
{
    lexbor_dobject_t dobj;
    lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    test_data_t *data = lexbor_dobject_calloc(&dobj);

    test_eq_size(data->a, 0UL);
    test_eq_char(data->b, 0x00);
    test_eq_int(data->c, 0);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(obj_mem_chunk)
{
    size_t count = 128;

    lexbor_dobject_t dobj;
    lexbor_dobject_init(&dobj, count, sizeof(test_data_t));

    for (size_t i = 0; i < count; i++) {
        lexbor_dobject_alloc(&dobj);
    }

    test_eq_size(lexbor_mem_chunk_length(dobj.mem), 1UL);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(obj_alloc_free_alloc)
{
    lexbor_dobject_t dobj;
    lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    test_data_t *data = lexbor_dobject_alloc(&dobj);

    data->a = 159753UL;
    data->b = 'L';
    data->c = 12;

    lexbor_dobject_free(&dobj, data);

    test_eq_size(lexbor_dobject_allocated(&dobj), 0UL);
    test_eq_size(lexbor_dobject_cache_length(&dobj), 1UL);

    data = lexbor_dobject_alloc(&dobj);

    test_eq_size(data->a, 159753UL);
    test_eq_char(data->b, 'L');
    test_eq_int(data->c, 12);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(obj_cache)
{
    lexbor_dobject_t dobj;

    size_t data_size;
    test_data_t *data[100];

    data_size = sizeof(data) / sizeof(*data);

    lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    for (size_t i = 0; i < data_size; i++) {
        data[i] = lexbor_dobject_alloc(&dobj);
        test_eq_size(lexbor_dobject_allocated(&dobj), (i + 1));
    }

    for (size_t i = 0; i < data_size; i++) {
        lexbor_dobject_free(&dobj, data[i]);
        test_eq_size(lexbor_dobject_cache_length(&dobj), (i + 1));
    }

    test_eq_size(lexbor_dobject_allocated(&dobj), 0UL);
    test_eq_size(lexbor_dobject_cache_length(&dobj), 100UL);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(absolute_position)
{
    test_data_t *data;
    lexbor_dobject_t dobj;

    lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    for (size_t i = 0; i < 100; i++) {
        data = lexbor_dobject_alloc(&dobj);

        data->a = i;
        data->b = (char) i;
        data->c = (int) i + 5;
    }

    data = lexbor_dobject_by_absolute_position(&dobj, 34);

    test_eq_size(data->a, 34UL);
    test_eq_char(data->b, (char) 34);
    test_eq_int(data->c, 39);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(absolute_position_up)
{
    test_data_t *data;
    lexbor_dobject_t dobj;

    lexbor_dobject_init(&dobj, 27, sizeof(test_data_t));

    for (size_t i = 0; i < 213; i++) {
        data = lexbor_dobject_alloc(&dobj);

        data->a = i;
        data->b = (char) i;
        data->c = (int) i + 5;
    }

    data = lexbor_dobject_by_absolute_position(&dobj, 121);

    test_eq_size(data->a, 121UL);
    test_eq_char(data->b, (char) 121);
    test_eq_int(data->c, 126);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(absolute_position_edge)
{
    test_data_t *data;
    lexbor_dobject_t dobj;

    lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    for (size_t i = 0; i < 256; i++) {
        data = lexbor_dobject_alloc(&dobj);

        data->a = i;
        data->b = (char) i;
        data->c = (int) i + 5;
    }

    data = lexbor_dobject_by_absolute_position(&dobj, 128);

    test_eq_size(data->a, 128UL);
    test_eq_char(data->b, (char) 128);
    test_eq_int(data->c, 133);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(obj_free)
{
    lexbor_dobject_t dobj;
    lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    test_data_t *data = lexbor_dobject_alloc(&dobj);
    lexbor_dobject_free(&dobj, data);

    test_eq_size(lexbor_dobject_allocated(&dobj), 0UL);
    test_eq_size(lexbor_dobject_cache_length(&dobj), 1UL);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_dobject_t dobj;
    lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    test_data_t *data = lexbor_dobject_alloc(&dobj);
    test_ne(data, NULL);

    lexbor_dobject_clean(&dobj);
    test_eq_size(lexbor_dobject_allocated(&dobj), 0UL);
    test_eq_size(lexbor_dobject_cache_length(&dobj), 0UL);

    lexbor_dobject_destroy(&dobj, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_dobject_t *dobj = lexbor_dobject_create();
    lexbor_dobject_init(dobj, 128, sizeof(test_data_t));

    test_eq(lexbor_dobject_destroy(dobj, true), NULL);

    dobj = lexbor_dobject_create();
    lexbor_dobject_init(dobj, 128, sizeof(test_data_t));

    test_eq(lexbor_dobject_destroy(dobj, false), dobj);
    test_eq(lexbor_dobject_destroy(dobj, true), NULL);
    test_eq(lexbor_dobject_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_dobject_t dobj;
    lexbor_dobject_init(&dobj, 128, sizeof(test_data_t));

    test_eq(lexbor_dobject_destroy(&dobj, false), &dobj);
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
    TEST_ADD(obj_alloc);
    TEST_ADD(obj_calloc);
    TEST_ADD(obj_mem_chunk);
    TEST_ADD(obj_alloc_free_alloc);
    TEST_ADD(obj_cache);
    TEST_ADD(absolute_position);
    TEST_ADD(absolute_position_up);
    TEST_ADD(absolute_position_edge);
    TEST_ADD(obj_free);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/dobject");
    TEST_RELEASE();
}
