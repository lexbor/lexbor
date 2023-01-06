/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/core/base.h"

static lexbor_malloc_f malloc_f = malloc;
static lexbor_realloc_f realloc_f = realloc;
static lexbor_calloc_f calloc_f = calloc;
static lexbor_free_f free_f = free;

void *
lexbor_malloc(size_t size)
{
    return malloc_f(size);
}

void *
lexbor_realloc(void *dst, size_t size)
{
    return realloc_f(dst, size);
}

void *
lexbor_calloc(size_t num, size_t size)
{
    return calloc_f(num, size);
}

void *
lexbor_free(void *dst)
{
    free_f(dst);
    return NULL;
}

lxb_status_t
lexbor_memory_setup(lexbor_malloc_f _malloc_f, lexbor_realloc_f _realloc_f,
                    lexbor_calloc_f _calloc_f, lexbor_free_f _free_f)
{
    if (_malloc_f == NULL || _realloc_f == NULL || _calloc_f == NULL || _free_f == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }
    malloc_f = _malloc_f;
    realloc_f = _realloc_f;
    calloc_f = _calloc_f;
    free_f = _free_f;
    return LXB_STATUS_OK;
}
