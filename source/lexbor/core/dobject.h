/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_DOBJECT_H
#define LEXBOR_DOBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"
#include "lexbor/core/mem.h"
#include "lexbor/core/array.h"

typedef struct lexbor_dobject_free_list_node {
    struct lexbor_dobject_free_list_node * next;
} lexbor_dobject_free_list_node_t;

typedef struct {
    lexbor_mem_t   *mem;
    lexbor_dobject_free_list_node_t *freelist;

    size_t         allocated;
    size_t         struct_size;
}
lexbor_dobject_t;


LXB_API lexbor_dobject_t *
lexbor_dobject_create(void);

LXB_API lxb_status_t
lexbor_dobject_init(lexbor_dobject_t *dobject,
                    size_t chunk_size, size_t struct_size);

LXB_API void
lexbor_dobject_clean(lexbor_dobject_t *dobject);

LXB_API lexbor_dobject_t *
lexbor_dobject_destroy(lexbor_dobject_t *dobject, bool destroy_self);


LXB_API void *
lexbor_dobject_alloc(lexbor_dobject_t *dobject);

LXB_API void *
lexbor_dobject_calloc(lexbor_dobject_t *dobject);

LXB_API void *
lexbor_dobject_free(lexbor_dobject_t *dobject, void *data);


LXB_API void *
lexbor_dobject_by_absolute_position(lexbor_dobject_t *dobject, size_t pos);


/*
 * Inline functions
 */
lxb_inline size_t
lexbor_dobject_allocated(lexbor_dobject_t *dobject)
{
    return dobject->allocated;
}

lxb_inline size_t
lexbor_dobject_cache_length(lexbor_dobject_t *dobject)
{
    // I am assuming this function is not performance critical
    // It used to be O(1) but now is O(n) in the length of the free-list

    size_t free_count = 0;
    lexbor_dobject_free_list_node_t *current_node = dobject->freelist;
    while (current_node != NULL) {
        free_count++;
        current_node = current_node->next;
    }

    return free_count;
}

/*
 * No inline functions for ABI.
 */
LXB_API size_t
lexbor_dobject_allocated_noi(lexbor_dobject_t *dobject);

LXB_API size_t
lexbor_dobject_cache_length_noi(lexbor_dobject_t *dobject);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOBJECT_H */


