/*
 * Copyright (C) 2018-2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/core/dobject.h"


#if defined(LEXBOR_HAVE_ADDRESS_SANITIZER)
    #include <sanitizer/asan_interface.h>
#endif


lexbor_dobject_t *
lexbor_dobject_create(void)
{
    return lexbor_calloc(1, sizeof(lexbor_dobject_t));
}

lxb_status_t
lexbor_dobject_init(lexbor_dobject_t *dobject,
                    size_t chunk_size, size_t struct_size)
{
    lxb_status_t status;

    if (dobject == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (chunk_size == 0 || struct_size == 0) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    if (struct_size < sizeof(lexbor_dobject_free_list_node_t)) {
        // raise struct size ensure there is enough room for the pointers in the free list
        struct_size = sizeof(lexbor_dobject_free_list_node_t);
    }

    /* Set params */
    dobject->allocated = 0UL;
    dobject->struct_size = struct_size;

    /* Init memory */
    dobject->mem = lexbor_mem_create();

    status = lexbor_mem_init(dobject->mem,
                           lexbor_mem_align(chunk_size * dobject->struct_size));
    if (status) {
        return status;
    }

#if defined(LEXBOR_HAVE_ADDRESS_SANITIZER)
    ASAN_POISON_MEMORY_REGION(dobject->mem->chunk->data,
                              dobject->mem->chunk->size);
#endif

    /* Setting up the free-list */
    dobject->freelist = NULL;

    return LXB_STATUS_OK;
}

void
lexbor_dobject_clean(lexbor_dobject_t *dobject)
{
    if (dobject != NULL) {
        dobject->allocated = 0UL;

        lexbor_mem_clean(dobject->mem);
        dobject->freelist = NULL;
    }
}

lexbor_dobject_t *
lexbor_dobject_destroy(lexbor_dobject_t *dobject, bool destroy_self)
{
    if (dobject == NULL)
        return NULL;

    dobject->mem = lexbor_mem_destroy(dobject->mem, true);

    if (destroy_self == true) {
        return lexbor_free(dobject);
    }

    return dobject;
}

void *
lexbor_dobject_alloc(lexbor_dobject_t *dobject)
{
    void *data;

    if (dobject->freelist != NULL) {
        dobject->allocated++;

        // pop the first node from the free list
        data = dobject->freelist;

#if defined(LEXBOR_HAVE_ADDRESS_SANITIZER)
        // unpoision the (re)allocated region
        ASAN_UNPOISON_MEMORY_REGION(data, dobject->struct_size);
#endif
        dobject->freelist = dobject->freelist->next;
        return data;
    }

    data = lexbor_mem_alloc(dobject->mem, dobject->struct_size);
    if (data == NULL) {
        return NULL;
    }

#if defined(LEXBOR_HAVE_ADDRESS_SANITIZER)
    ASAN_UNPOISON_MEMORY_REGION(data, dobject->struct_size);
#endif

    dobject->allocated++;

    return data;
}

void *
lexbor_dobject_calloc(lexbor_dobject_t *dobject)
{
    void *data = lexbor_dobject_alloc(dobject);

    if (data != NULL) {
        memset(data, 0, dobject->struct_size);
    }

    return data;
}

void *
lexbor_dobject_free(lexbor_dobject_t *dobject, void *data)
{
    if (data == NULL) {
        return NULL;
    }

    // insert newly freed object slot to the head of the free list
    dobject->allocated--;
    lexbor_dobject_free_list_node_t *new_node = (lexbor_dobject_free_list_node_t *)data;
    new_node->next = dobject->freelist;
    dobject->freelist = new_node;

#if defined(LEXBOR_HAVE_ADDRESS_SANITIZER)
    // poison the freed region
    ASAN_POISON_MEMORY_REGION(data, dobject->struct_size);
#endif

    return NULL;
}

void *
lexbor_dobject_by_absolute_position(lexbor_dobject_t *dobject, size_t pos)
{
    size_t chunk_idx, chunk_pos, i;
    lexbor_mem_chunk_t *chunk;

    if (pos >= dobject->allocated) {
        return NULL;
    }

    chunk = dobject->mem->chunk_first;
    chunk_pos = pos * dobject->struct_size;
    chunk_idx = chunk_pos / dobject->mem->chunk_min_size;

    for (i = 0; i < chunk_idx; i++) {
        chunk = chunk->next;
    }

    return &chunk->data[chunk_pos % chunk->size];
}

/*
 * No inline functions for ABI.
 */
size_t
lexbor_dobject_allocated_noi(lexbor_dobject_t *dobject)
{
    return lexbor_dobject_allocated(dobject);
}

size_t
lexbor_dobject_cache_length_noi(lexbor_dobject_t *dobject)
{
    return lexbor_dobject_cache_length(dobject);
}
