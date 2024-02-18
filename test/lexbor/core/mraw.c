/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/mraw.h>


TEST_BEGIN(init)
{
    lexbor_mraw_t *mraw = lexbor_mraw_create();
    lxb_status_t status = lexbor_mraw_init(mraw, 1024);

    test_eq(status, LXB_STATUS_OK);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_mraw_init(NULL, 1024);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_mraw_t mraw;
    lxb_status_t status = lexbor_mraw_init(&mraw, 1024);

    test_eq(status, LXB_STATUS_OK);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_mraw_t mraw = {0};
    lxb_status_t status;

    status = lexbor_mraw_init(&mraw, 0);
    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_alloc)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    void *data = lexbor_mraw_alloc(&mraw, 127);
    test_ne(data, NULL);

    test_eq_size(lexbor_mraw_data_size(data), lexbor_mem_align(127));

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length,
                 lexbor_mem_align(127) + lexbor_mraw_meta_size());

    test_eq_size(mraw.mem->chunk->size,
                 lexbor_mem_align(1024) + lexbor_mraw_meta_size());

    test_eq_size(mraw.cache->tree_length, 0UL);

    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_alloc_eq)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    void *data = lexbor_mraw_alloc(&mraw, 1024);
    test_ne(data, NULL);

    test_eq_size(lexbor_mraw_data_size(data), 1024UL);

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length, 1024UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.cache->tree_length, 0UL);

    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_alloc_overflow_if_len_0)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    void *data = lexbor_mraw_alloc(&mraw, 1025);
    test_ne(data, NULL);

    test_eq_size(lexbor_mraw_data_size(data), lexbor_mem_align(1025));

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length,
                 lexbor_mem_align(1025) + lexbor_mraw_meta_size());

    test_eq_size(mraw.mem->chunk->size,
                 lexbor_mem_align(1025) + lexbor_mem_align(1024)
                 + (2 * lexbor_mraw_meta_size()));

    test_eq_size(mraw.cache->tree_length, 0UL);
    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_alloc_overflow_if_len_not_0)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    void *data = lexbor_mraw_alloc(&mraw, 13);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), lexbor_mem_align(13));

    data = lexbor_mraw_alloc(&mraw, 1025);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), lexbor_mem_align(1025));

    test_eq_size(mraw.mem->chunk_first->length, 1024UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk_first->size, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.mem->chunk_length, 2UL);
    test_eq_size(mraw.mem->chunk->length,
                 lexbor_mem_align(1025) + lexbor_mraw_meta_size());

    test_eq_size(mraw.mem->chunk->size,
                 lexbor_mem_align(1025) + lexbor_mem_align(1024)
                 + (2 * lexbor_mraw_meta_size()));

    test_eq_size(mraw.cache->tree_length, 1UL);
    test_eq_size(mraw.cache->root->size,
                 (lexbor_mem_align(1024) + lexbor_mraw_meta_size())  /* Init size. */
                 - (lexbor_mem_align(13) + lexbor_mraw_meta_size()) /* First alloc size. */
                 - lexbor_mraw_meta_size());                        /* Insert meta before
                                                                     * append to cache.
                                                                     */
    test_ne(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_alloc_if_len_not_0)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    void *data = lexbor_mraw_alloc(&mraw, 8);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), lexbor_mem_align(8));

    data = lexbor_mraw_alloc(&mraw, 1016 - lexbor_mraw_meta_size());
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), 1016 - lexbor_mraw_meta_size());

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length, 1024UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk->size, mraw.mem->chunk->length);

    test_eq_size(mraw.cache->tree_length, 0UL);
    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *data = lexbor_mraw_alloc(&mraw, 128);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), 128UL);

    uint8_t *new_data = lexbor_mraw_realloc(&mraw, data, 256);
    test_ne(new_data, NULL);
    test_eq_size(lexbor_mraw_data_size(new_data), 256UL);

    test_eq(data, new_data);

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length, 256UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.cache->tree_length, 0UL);
    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc_eq)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *data = lexbor_mraw_alloc(&mraw, 128);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), 128UL);

    uint8_t *new_data = lexbor_mraw_realloc(&mraw, data, 128);
    test_ne(new_data, NULL);
    test_eq_size(lexbor_mraw_data_size(new_data), 128UL);

    test_eq(data, new_data);

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length, 128UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.cache->tree_length, 0UL);
    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc_tail_0)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *data = lexbor_mraw_alloc(&mraw, 128);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), 128UL);

    uint8_t *new_data = lexbor_mraw_realloc(&mraw, data, 0);
    test_eq(new_data, NULL);

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length, 0UL);
    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.cache->tree_length, 0UL);
    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc_tail_n)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *data = lexbor_mraw_alloc(&mraw, 128);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), 128UL);

    data = lexbor_mraw_alloc(&mraw, 128);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), 128UL);

    uint8_t *new_data = lexbor_mraw_realloc(&mraw, data, 1024);
    test_ne(new_data, NULL);
    test_eq_size(lexbor_mraw_data_size(new_data), 1024UL);

    test_ne(data, new_data);

    test_eq_size(mraw.mem->chunk_length, 2UL);
    test_eq_size(mraw.mem->chunk->length, 1024UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.cache->tree_length, 1UL);
    test_eq_size(mraw.cache->root->size, (1024 + lexbor_mraw_meta_size())
                 - (128UL + lexbor_mraw_meta_size())
                 - lexbor_mraw_meta_size());

    test_ne(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc_tail_less)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *data = lexbor_mraw_alloc(&mraw, 128);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), 128UL);

    uint8_t *new_data = lexbor_mraw_realloc(&mraw, data, 16);
    test_ne(new_data, NULL);
    test_eq_size(lexbor_mraw_data_size(new_data), 16UL);

    test_eq(data, new_data);

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length, 16UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.cache->tree_length, 0UL);
    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc_tail_great)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *data = lexbor_mraw_alloc(&mraw, 128);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), 128UL);

    uint8_t *new_data = lexbor_mraw_realloc(&mraw, data, 2046);
    test_ne(new_data, NULL);
    test_eq_size(lexbor_mraw_data_size(new_data), lexbor_mem_align(2046));

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length,
                 lexbor_mem_align(2046) + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk->size,
                 lexbor_mem_align(2046) + 1024UL + (2 * lexbor_mraw_meta_size()));

    test_eq_size(mraw.cache->tree_length, 0UL);
    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc_n)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *one = lexbor_mraw_alloc(&mraw, 128);
    test_ne(one, NULL);
    test_eq_size(lexbor_mraw_data_size(one), 128UL);

    uint8_t *two = lexbor_mraw_alloc(&mraw, 13);
    test_ne(two, NULL);
    test_eq_size(lexbor_mraw_data_size(two), lexbor_mem_align(13));

    uint8_t *three = lexbor_mraw_realloc(&mraw, one, 256);
    test_ne(three, NULL);
    test_eq_size(lexbor_mraw_data_size(three), 256UL);

    test_ne(one, three);

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length, 128UL + lexbor_mraw_meta_size()
                 + lexbor_mem_align(13) + lexbor_mraw_meta_size()
                 + 256UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.cache->tree_length, 1UL);
    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    test_ne(mraw.cache->root, NULL);
    test_eq_size(mraw.cache->root->size, 128UL);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc_n_0)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *one = lexbor_mraw_alloc(&mraw, 128);
    test_ne(one, NULL);
    test_eq_size(lexbor_mraw_data_size(one), 128UL);

    uint8_t *two = lexbor_mraw_alloc(&mraw, 13);
    test_ne(two, NULL);
    test_eq_size(lexbor_mraw_data_size(two), lexbor_mem_align(13));

    uint8_t *three = lexbor_mraw_realloc(&mraw, one, 0);
    test_eq(three, NULL);

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->length, 128UL + lexbor_mraw_meta_size()
                 + lexbor_mem_align(13) + lexbor_mraw_meta_size());

    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.cache->tree_length, 1UL);
    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    test_ne(mraw.cache->root, NULL);
    test_eq_size(mraw.cache->root->size, 128UL);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc_n_less)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *one = lexbor_mraw_alloc(&mraw, 128);
    test_ne(one, NULL);
    test_eq_size(lexbor_mraw_data_size(one), 128UL);

    uint8_t *two = lexbor_mraw_alloc(&mraw, 256);
    test_ne(two, NULL);
    test_eq_size(lexbor_mraw_data_size(two), 256UL);

    uint8_t *three = lexbor_mraw_realloc(&mraw, one, 51);
    test_ne(three, NULL);
    test_eq_size(lexbor_mraw_data_size(three), lexbor_mem_align(51));

    test_eq(one, three);

    test_eq_size(mraw.cache->tree_length, 1UL);
    test_ne(mraw.cache->root, NULL);

    test_eq_size(mraw.cache->root->size, (128UL + lexbor_mraw_meta_size())
                 - (lexbor_mem_align(51) + lexbor_mraw_meta_size())
                 - lexbor_mraw_meta_size());

    test_eq_size(mraw.mem->chunk_length, 1UL);
    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk->length, 128UL + lexbor_mraw_meta_size()
                 + 256UL + lexbor_mraw_meta_size());

    test_eq(mraw.mem->chunk, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_realloc_n_great)
{
    lexbor_mraw_t mraw = {0};
    lexbor_bst_entry_t *cache_entry;

    lexbor_mraw_init(&mraw, 1024);

    uint8_t *one = lexbor_mraw_alloc(&mraw, 128);
    test_ne(one, NULL);
    test_eq_size(lexbor_mraw_data_size(one), 128UL);

    uint8_t *two = lexbor_mraw_alloc(&mraw, 256);
    test_ne(two, NULL);
    test_eq_size(lexbor_mraw_data_size(two), 256UL);

    uint8_t *three = lexbor_mraw_realloc(&mraw, one, 1000);
    test_ne(three, NULL);
    test_eq_size(lexbor_mraw_data_size(three), 1000UL);

    test_ne(one, three);

    test_eq_size(mraw.cache->tree_length, 2UL);
    test_ne(mraw.cache->root, NULL);

    cache_entry = lexbor_bst_search(mraw.cache, mraw.cache->root, 128UL);
    test_ne(cache_entry, NULL);

    size_t size = (1024 + lexbor_mraw_meta_size())
    - (128UL + lexbor_mraw_meta_size())
    - (256UL + lexbor_mraw_meta_size())
    - lexbor_mraw_meta_size();

    cache_entry = lexbor_bst_search(mraw.cache, mraw.cache->root, size);
    test_ne(cache_entry, NULL);

    test_eq_size(mraw.mem->chunk_length, 2UL);
    test_eq_size(mraw.mem->chunk_first->size, 1024UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk_first->length, 1024UL + lexbor_mraw_meta_size());

    test_eq_size(mraw.mem->chunk->size, 1024UL + lexbor_mraw_meta_size());
    test_eq_size(mraw.mem->chunk->length, 1000UL + lexbor_mraw_meta_size());

    test_ne(mraw.mem->chunk, mraw.mem->chunk_first);

    test_eq(mraw.mem->chunk_first->next, mraw.mem->chunk);
    test_eq(mraw.mem->chunk->prev, mraw.mem->chunk_first);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_free)
{
    lexbor_mraw_t mraw = {0};
    lexbor_bst_entry_t *cache_entry;

    lexbor_mraw_init(&mraw, 1024);

    uint8_t *data = lexbor_mraw_calloc(&mraw, 23);
    test_ne(data, NULL);

    lexbor_mraw_free(&mraw, data);

    cache_entry = lexbor_bst_search(mraw.cache, mraw.cache->root,
                                    lexbor_mem_align(23));
    test_ne(cache_entry, NULL);

    cache_entry = lexbor_bst_search_close(mraw.cache, mraw.cache->root, 23);
    test_ne(cache_entry, NULL);
    test_eq_size(cache_entry->size, lexbor_mem_align(23));

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(mraw_calloc)
{
    lexbor_mraw_t mraw = {0};
    lexbor_mraw_init(&mraw, 1024);

    uint8_t *data = lexbor_mraw_calloc(&mraw, 1024);
    test_ne(data, NULL);
    test_eq_size(lexbor_mraw_data_size(data), 1024UL);

    for (size_t i = 0; i < 1024; i++) {
        test_eq(data[i], 0x00);
    }

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_mraw_t mraw;
    lexbor_mraw_init(&mraw, 1024);

    lexbor_mraw_clean(&mraw);

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_mraw_t *mraw = lexbor_mraw_create();
    lexbor_mraw_init(mraw, 1024);

    test_eq(lexbor_mraw_destroy(mraw, true), NULL);

    mraw = lexbor_mraw_create();
    lexbor_mraw_init(mraw, 1021);

    test_eq(lexbor_mraw_destroy(mraw, false), mraw);
    test_eq(lexbor_mraw_destroy(mraw, true), NULL);
    test_eq(lexbor_mraw_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_mraw_t mraw;
    lexbor_mraw_init(&mraw, 1023);

    test_eq(lexbor_mraw_destroy(&mraw, false), &mraw);
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
    TEST_ADD(mraw_alloc);
    TEST_ADD(mraw_alloc_eq);
    TEST_ADD(mraw_alloc_overflow_if_len_0);
    TEST_ADD(mraw_alloc_overflow_if_len_not_0);
    TEST_ADD(mraw_alloc_if_len_not_0);
    TEST_ADD(mraw_realloc);
    TEST_ADD(mraw_realloc_eq);
    TEST_ADD(mraw_realloc_tail_0);
    TEST_ADD(mraw_realloc_tail_n);
    TEST_ADD(mraw_realloc_tail_less);
    TEST_ADD(mraw_realloc_tail_great);
    TEST_ADD(mraw_realloc_n);
    TEST_ADD(mraw_realloc_n_0);
    TEST_ADD(mraw_realloc_n_less);
    TEST_ADD(mraw_realloc_n_great);
    TEST_ADD(mraw_calloc);
    TEST_ADD(mraw_free);
    TEST_ADD(clean);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/mraw");
    TEST_RELEASE();
}
