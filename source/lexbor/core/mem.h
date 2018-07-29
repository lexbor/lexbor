/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_MEM_H
#define LEXBOR_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "lexbor/core/base.h"


typedef struct lexbor_mem_chunk lexbor_mem_chunk_t;
typedef struct lexbor_mem lexbor_mem_t;

struct lexbor_mem_chunk {
    uint8_t            *data;
    size_t             length;
    size_t             size;

    lexbor_mem_chunk_t *next;
    lexbor_mem_chunk_t *prev;
};

struct lexbor_mem {
    lexbor_mem_chunk_t *chunk;
    lexbor_mem_chunk_t *chunk_first;

    size_t             chunk_min_size;
    size_t             chunk_length;
};


lexbor_mem_t *
lexbor_mem_create(void);

lxb_status_t
lexbor_mem_init(lexbor_mem_t *mem, size_t min_chunk_size);

void
lexbor_mem_clean(lexbor_mem_t *mem);

lexbor_mem_t *
lexbor_mem_destroy(lexbor_mem_t *mem, bool destroy_self);


uint8_t *
lexbor_mem_chunk_init(lexbor_mem_t *mem,
                      lexbor_mem_chunk_t *chunk, size_t length);

lexbor_mem_chunk_t *
lexbor_mem_chunk_make(lexbor_mem_t *mem, size_t length);

lexbor_mem_chunk_t *
lexbor_mem_chunk_destroy(lexbor_mem_t *mem,
                         lexbor_mem_chunk_t *chunk, bool self_destroy);


void *
lexbor_mem_alloc(lexbor_mem_t *mem, size_t length);

void *
lexbor_mem_calloc(lexbor_mem_t *mem, size_t length);


/*
 * Inline functions
 */
lxb_inline size_t
lexbor_mem_current_length(lexbor_mem_t *mem)
{
    return mem->chunk->length;
}

lxb_inline size_t
lexbor_mem_current_size(lexbor_mem_t *mem)
{
    return mem->chunk->size;
}

lxb_inline size_t
lexbor_mem_chunk_length(lexbor_mem_t *mem)
{
    return mem->chunk_length;
}

lxb_inline size_t
lexbor_mem_align(size_t size)
{
    return ((size % LEXBOR_MEM_ALIGN_STEP) != 0)
           ? size + (LEXBOR_MEM_ALIGN_STEP - (size % LEXBOR_MEM_ALIGN_STEP))
           : size;
}

lxb_inline size_t
lexbor_mem_align_floor(size_t size)
{
    return ((size % LEXBOR_MEM_ALIGN_STEP) != 0)
           ? size - (size % LEXBOR_MEM_ALIGN_STEP)
           : size;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_MEM_H */
