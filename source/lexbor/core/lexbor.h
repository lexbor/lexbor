/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_H
#define LEXBOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/def.h"

typedef void *(*lexbor_malloc_f)(size_t size);
typedef void *(*lexbor_realloc_f)(void *dst, size_t size);
typedef void *(*lexbor_calloc_f)(size_t num, size_t size);
typedef void (*lexbor_free_f)(void *dst);

LXB_API void *
lexbor_malloc(size_t size);

LXB_API void *
lexbor_realloc(void *dst, size_t size);

LXB_API void *
lexbor_calloc(size_t num, size_t size);

LXB_API void *
lexbor_free(void *dst);

LXB_API lxb_status_t
lexbor_memory_setup(lexbor_malloc_f malloc_f, lexbor_realloc_f realloc_f,
                    lexbor_calloc_f calloc_f, lexbor_free_f free_f);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_H */

