/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_MRAW_H
#define LEXBOR_MRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "lexbor/core/base.h"
#include "lexbor/core/mem.h"
#include "lexbor/core/bst.h"


#define lexbor_mraw_meta_size()                                                \
    (((sizeof(size_t) % LEXBOR_MEM_ALIGN_STEP) != 0)                           \
    ? sizeof(size_t)                                                           \
        + (LEXBOR_MEM_ALIGN_STEP - (sizeof(size_t) % LEXBOR_MEM_ALIGN_STEP))   \
    : sizeof(size_t))


typedef struct {
    lexbor_mem_t *mem;
    lexbor_bst_t *cache;
}
lexbor_mraw_t;


lexbor_mraw_t *
lexbor_mraw_create(void);

lxb_status_t
lexbor_mraw_init(lexbor_mraw_t *mraw, size_t chunk_size);

void
lexbor_mraw_clean(lexbor_mraw_t *mraw);

lexbor_mraw_t *
lexbor_mraw_destroy(lexbor_mraw_t *mraw, bool destroy_self);


void *
lexbor_mraw_alloc(lexbor_mraw_t *mraw, size_t size);

void *
lexbor_mraw_calloc(lexbor_mraw_t *mraw, size_t size);

void *
lexbor_mraw_realloc(lexbor_mraw_t *mraw, void *data, size_t new_size);

void *
lexbor_mraw_free(lexbor_mraw_t *mraw, void *data);


/*
 * Inline functions
 */
lxb_inline size_t
lexbor_mraw_data_size(void *data)
{
    return *((size_t *) (((uint8_t *) data) - lexbor_mraw_meta_size()));
}

lxb_inline void
lexbor_mraw_data_size_set(void *data, size_t size)
{
    data = (((uint8_t *) data) - lexbor_mraw_meta_size());
    memcpy(data, &size, sizeof(size_t));
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_MRAW_H */
