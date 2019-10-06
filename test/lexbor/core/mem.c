/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/mem.h>


TEST_BEGIN(init)
{
    lexbor_mem_t *mem = lexbor_mem_create();
    lxb_status_t status = lexbor_mem_init(mem, 1024);

    test_eq(status, LXB_STATUS_OK);

    test_ne(mem->chunk, NULL);
    test_eq(mem->chunk, mem->chunk_first);

    test_ne(mem->chunk->data, NULL);
    test_eq(mem->chunk->next, NULL);
    test_eq(mem->chunk->prev, NULL);

    test_eq_size(mem->chunk->length, 0UL);
    test_eq_size(mem->chunk->size, 1024UL);

    test_eq_size(mem->chunk_length, 1UL);
    test_eq_size(mem->chunk_min_size, 1024UL);

    lexbor_mem_destroy(mem, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_mem_init(NULL, 1024);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_mem_t mem;
    lxb_status_t status = lexbor_mem_init(&mem, 1024);

    test_eq(status, LXB_STATUS_OK);

    lexbor_mem_destroy(&mem, false);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_mem_t mem = {0};
    lxb_status_t status;

    status = lexbor_mem_init(&mem, 0);
    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    lexbor_mem_destroy(&mem, false);
}
TEST_END

TEST_BEGIN(mem_alloc)
{
    lexbor_mem_t mem;
    lexbor_mem_init(&mem, 1024);

    void *data = lexbor_mem_alloc(&mem, 12);
    test_ne(data, NULL);

    lexbor_mem_destroy(&mem, false);
}
TEST_END

TEST_BEGIN(mem_alloc_n)
{
    lexbor_mem_t mem;
    lexbor_mem_init(&mem, 1024);

    for (size_t i = 0; i < 32; i++) {
        test_ne(lexbor_mem_alloc(&mem, 32), NULL);
    }

    test_eq_size(mem.chunk_length, 1UL);

    lexbor_mem_destroy(&mem, false);
}
TEST_END

TEST_BEGIN(mem_alloc_overflow)
{
    lexbor_mem_t mem;
    lexbor_mem_init(&mem, 31);

    void *data = lexbor_mem_alloc(&mem, 1047);
    test_ne(data, NULL);

    test_eq_size(mem.chunk_length, 2UL);

    lexbor_mem_destroy(&mem, false);
}
TEST_END

TEST_BEGIN(mem_calloc)
{
    size_t len = 12;

    lexbor_mem_t mem;
    lexbor_mem_init(&mem, 1024);

    uint8_t *data = lexbor_mem_calloc(&mem, len);
    test_ne(data, NULL);

    for (size_t i = 0; i < len; i++) {
        test_eq(data[i], 0x00);
    }

    test_eq_size(mem.chunk_length, 1UL);

    lexbor_mem_destroy(&mem, false);
}
TEST_END

TEST_BEGIN(mem_calloc_overflow)
{
    size_t len = 1027;

    lexbor_mem_t mem;
    lexbor_mem_init(&mem, 31);

    uint8_t *data = lexbor_mem_calloc(&mem, len);
    test_ne(data, NULL);

    for (size_t i = 0; i < len; i++) {
        test_eq(data[i], 0x00);
    }

    test_eq_size(mem.chunk_length, 2UL);

    lexbor_mem_destroy(&mem, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_mem_t mem;
    lexbor_mem_init(&mem, 12);

    for (size_t i = 0; i < 32; i++) {
        test_ne(lexbor_mem_alloc(&mem, 24), NULL);
    }

    lexbor_mem_clean(&mem);

    test_eq_size(mem.chunk_length, 1UL);

    test_ne(mem.chunk, NULL);
    test_eq(mem.chunk_first, mem.chunk);

    test_eq_size(mem.chunk->length, 0UL);
    test_eq_size(mem.chunk->size, mem.chunk_min_size);

    lexbor_mem_destroy(&mem, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_mem_t *mem = lexbor_mem_create();
    lexbor_mem_init(mem, 1024);

    test_eq(lexbor_mem_destroy(mem, true), NULL);

    mem = lexbor_mem_create();
    lexbor_mem_init(mem, 1021);

    test_eq(lexbor_mem_destroy(mem, false), mem);
    test_eq(lexbor_mem_destroy(mem, true), NULL);
    test_eq(lexbor_mem_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_mem_t mem;
    lexbor_mem_init(&mem, 1023);

    test_eq(lexbor_mem_destroy(&mem, false), &mem);
}
TEST_END

TEST_BEGIN(chunk_init)
{
    lexbor_mem_chunk_t chunk;

    lexbor_mem_t *mem = lexbor_mem_create();
    lexbor_mem_init(mem, 1024);

    uint8_t *chunk_data = lexbor_mem_chunk_init(mem, &chunk, 0);
    test_ne(chunk_data, NULL);

    test_ne(chunk.data, NULL);
    test_eq_size(chunk.length, 0UL);
    test_eq_size(chunk.size, mem->chunk_min_size);

    lexbor_mem_chunk_destroy(mem, &chunk, false);
    lexbor_mem_destroy(mem, true);
}
TEST_END

TEST_BEGIN(chunk_init_overflow)
{
    lexbor_mem_chunk_t chunk;

    lexbor_mem_t *mem = lexbor_mem_create();
    lexbor_mem_init(mem, 1024);

    uint8_t *chunk_data = lexbor_mem_chunk_init(mem, &chunk, 2049);
    test_ne(chunk_data, NULL);

    test_ne(chunk.data, NULL);
    test_eq_size(chunk.length, 0UL);
    test_eq_size(chunk.size, lexbor_mem_align(2049) +  lexbor_mem_align(1024));

    lexbor_mem_chunk_destroy(mem, &chunk, false);
    lexbor_mem_destroy(mem, true);
}
TEST_END

TEST_BEGIN(chunk_make)
{
    lexbor_mem_t *mem = lexbor_mem_create();
    lexbor_mem_init(mem, 1024);

    lexbor_mem_chunk_t *chunk = lexbor_mem_chunk_make(mem, 0);
    test_ne(chunk, NULL);

    test_ne(chunk->data, NULL);
    test_eq_size(chunk->length, 0UL);
    test_eq_size(chunk->size, mem->chunk_min_size);

    lexbor_mem_chunk_destroy(mem, chunk, true);
    lexbor_mem_destroy(mem, true);
}
TEST_END

TEST_BEGIN(chunk_make_overflow)
{
    lexbor_mem_t *mem = lexbor_mem_create();
    lexbor_mem_init(mem, 1024);

    lexbor_mem_chunk_t *chunk = lexbor_mem_chunk_make(mem, 2049);
    test_ne(chunk, NULL);

    test_ne(chunk->data, NULL);
    test_eq_size(chunk->length, 0UL);
    test_eq_size(chunk->size, lexbor_mem_align(2049) + lexbor_mem_align(1024));

    lexbor_mem_chunk_destroy(mem, chunk, true);
    lexbor_mem_destroy(mem, true);
}
TEST_END

TEST_BEGIN(chunk_destroy)
{
    lexbor_mem_chunk_t *chunk, *chunk_null;

    lexbor_mem_t *mem = lexbor_mem_create();
    lexbor_mem_init(mem, 1024);

    chunk = lexbor_mem_chunk_make(mem, 0);
    chunk = lexbor_mem_chunk_destroy(mem, chunk, true);

    test_eq(chunk, NULL);

    chunk = lexbor_mem_chunk_make(mem, 0);
    chunk = lexbor_mem_chunk_destroy(mem, chunk, false);

    test_ne(chunk, NULL);
    test_eq(chunk->data, NULL);

    chunk = lexbor_mem_chunk_destroy(mem, chunk, true);

    chunk = lexbor_mem_chunk_make(mem, 0);
    test_ne(chunk, NULL);

    chunk_null = lexbor_mem_chunk_destroy(NULL, chunk, false);
    test_eq(chunk_null, NULL);

    chunk = lexbor_mem_chunk_destroy(mem, chunk, true);
    test_eq(chunk, NULL);

    chunk = lexbor_mem_chunk_destroy(mem, NULL, false);
    test_eq(chunk, NULL);

    chunk = lexbor_mem_chunk_destroy(NULL, NULL, false);
    test_eq(chunk, NULL);

    lexbor_mem_destroy(mem, true);
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
    TEST_ADD(mem_alloc);
    TEST_ADD(mem_alloc_n);
    TEST_ADD(mem_alloc_overflow);
    TEST_ADD(mem_calloc);
    TEST_ADD(mem_calloc_overflow);
    TEST_ADD(clean);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_ADD(chunk_init);
    TEST_ADD(chunk_init_overflow);
    TEST_ADD(chunk_make);
    TEST_ADD(chunk_make_overflow);
    TEST_ADD(chunk_destroy);

    TEST_RUN("lexbor/core/mem");
    TEST_RELEASE();
}
