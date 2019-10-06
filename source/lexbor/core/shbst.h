/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_SHBST_H
#define LEXBOR_SHBST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"
#include "lexbor/core/bst.h"
#include "lexbor/core/mraw.h"


typedef struct {
    lxb_char_t *key;
    void       *value;
}
lexbor_shbst_entry_t;

typedef struct {
    lexbor_bst_t       *bst;
    lexbor_mraw_t      *keys;
    lexbor_dobject_t   *entries;

    lexbor_bst_entry_t **table;
    size_t             table_size;
}
lexbor_shbst_t;


LXB_API lexbor_shbst_t *
lexbor_shbst_create(void);

LXB_API lxb_status_t
lexbor_shbst_init(lexbor_shbst_t *shbst, size_t table_size);

LXB_API void
lexbor_shbst_clean(lexbor_shbst_t *shbst);

LXB_API lexbor_shbst_t *
lexbor_shbst_destroy(lexbor_shbst_t *shbst, bool self_destroy);


LXB_API lexbor_shbst_entry_t *
lexbor_shbst_search(lexbor_shbst_t *shbst, const lxb_char_t *key,
                    size_t key_size, bool case_insensitive);

LXB_API lexbor_shbst_entry_t *
lexbor_shbst_insert(lexbor_shbst_t *shbst, const lxb_char_t *key,
                    size_t key_size, void *value);

LXB_API lexbor_shbst_entry_t *
lexbor_shbst_insert_lowercase(lexbor_shbst_t *shbst, const lxb_char_t *key,
                              size_t key_size, void *value);

/*
 * Insert a key without copying.
 * Key should always be created using 'keys' in lexbor_shbst_t structure.
 */
LXB_API lexbor_shbst_entry_t *
lexbor_shbst_insert_wo_copy(lexbor_shbst_t *shbst,
                            lxb_char_t *key, size_t key_size, void *value);

LXB_API lexbor_shbst_entry_t *
lexbor_shbst_replace(lexbor_shbst_t *shbst,
                     const lxb_char_t *key, size_t key_size,
                     void *value, bool case_insensitive);

/*
 * Inline functions
 */
lxb_inline lexbor_mraw_t *
lexbor_shbst_keys(lexbor_shbst_t *shbst)
{
    return shbst->keys;
}

/*
 * No inline functions for ABI.
 */
lexbor_mraw_t *
lexbor_shbst_keys_noi(lexbor_shbst_t *shbst);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_SHBST_H */
