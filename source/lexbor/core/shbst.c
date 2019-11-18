/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/core/shbst.h"
#include "lexbor/core/str.h"

#define LEXBOR_STR_RES_MAP_LOWERCASE
#define LEXBOR_STR_RES_ANSI_REPLACEMENT_CHARACTER
#include "lexbor/core/str_res.h"


#define lexbor_shbst_make_id_m(shbst, key, size)                               \
    (((((lexbor_str_res_map_lowercase[key[0]]                                  \
     * lexbor_str_res_map_lowercase[key[size - 1]])                            \
     * lexbor_str_res_map_lowercase[key[0]])                                   \
     + size)                                                                   \
     % shbst->table_size) + 0x01)


typedef bool (*lexbor_shbst_cmp_f)(const lxb_char_t *str1,
                                   const lxb_char_t *str2, size_t size);


lxb_inline lexbor_shbst_entry_t *
lexbor_shbst_entry_create(lexbor_shbst_t *shbst);

static lexbor_shbst_entry_t *
lexbor_shbst_entry_make(lexbor_shbst_t *shbst, const lxb_char_t *key,
                        size_t key_size, void *value);

static lexbor_shbst_entry_t *
lexbor_shbst_entry_make_lowercase(lexbor_shbst_t *shbst, const lxb_char_t *key,
                                  size_t key_size, void *value);

static lexbor_shbst_entry_t *
lexbor_shbst_entry_destroy(lexbor_shbst_t *shbst,
                           lexbor_shbst_entry_t *entry);

static lexbor_shbst_entry_t *
lexbor_shbst_search_sbst(lexbor_shbst_t *shbst, lexbor_bst_entry_t *scope,
                         lexbor_shbst_cmp_f cmp_f,
                         const lxb_char_t *key, size_t key_size);


lexbor_shbst_t *
lexbor_shbst_create(void)
{
    return lexbor_calloc(1, sizeof(lexbor_shbst_t));
}

lxb_status_t
lexbor_shbst_init(lexbor_shbst_t *shbst, size_t table_size)
{
    lxb_status_t status;

    if (shbst == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (table_size == 0) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    /* Create SBST */
    shbst->bst = lexbor_bst_create();
    status = lexbor_bst_init(shbst->bst, 128);

    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Create mraw for keys */
    shbst->keys = lexbor_mraw_create();
    status = lexbor_mraw_init(shbst->keys, 2048);

    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Create DObject for entries */
    shbst->entries = lexbor_dobject_create();
    status = lexbor_dobject_init(shbst->entries, 128,
                                 sizeof(lexbor_shbst_entry_t));

    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Create table */
    shbst->table = lexbor_calloc(table_size + 1, sizeof(void *));
    if (shbst->table == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    shbst->table_size = table_size;

    return LXB_STATUS_OK;
}

void
lexbor_shbst_clean(lexbor_shbst_t *shbst)
{
    lexbor_bst_clean(shbst->bst);
    lexbor_mraw_clean(shbst->keys);
    lexbor_dobject_clean(shbst->entries);

    memset(shbst->table, 0, sizeof(void *) * (shbst->table_size + 1));
}

lexbor_shbst_t *
lexbor_shbst_destroy(lexbor_shbst_t *shbst, bool self_destroy)
{
    if (shbst == NULL) {
        return NULL;
    }

    shbst->bst = lexbor_bst_destroy(shbst->bst, true);
    shbst->keys = lexbor_mraw_destroy(shbst->keys, true);
    shbst->entries = lexbor_dobject_destroy(shbst->entries, true);

    if (shbst->table != NULL) {
        shbst->table = lexbor_free(shbst->table);
    }

    if (self_destroy == true) {
        return lexbor_free(shbst);
    }

    return shbst;
}

lxb_inline lexbor_shbst_entry_t *
lexbor_shbst_entry_create(lexbor_shbst_t *shbst)
{
    return lexbor_dobject_alloc(shbst->entries);
}

static lexbor_shbst_entry_t *
lexbor_shbst_entry_make(lexbor_shbst_t *shbst,
                        const lxb_char_t *key, size_t key_size, void *value)
{
    lexbor_shbst_entry_t *entry = lexbor_shbst_entry_create(shbst);

    if (entry == NULL) {
        return NULL;
    }

    if (key != NULL) {
        entry->key = lexbor_mraw_alloc(shbst->keys, (key_size + 1));
        if (entry->key == NULL) {
            return NULL;
        }

        memcpy(entry->key, key, sizeof(lxb_char_t) * key_size);

        entry->key[key_size] = '\0';
    }
    else {
        entry->key = NULL;
    }

    entry->value = value;

    return entry;
}

static lexbor_shbst_entry_t *
lexbor_shbst_entry_make_lowercase(lexbor_shbst_t *shbst, const lxb_char_t *key,
                                  size_t key_size, void *value)
{
    lexbor_shbst_entry_t *entry = lexbor_shbst_entry_create(shbst);

    if (entry == NULL) {
        return NULL;
    }

    if (key != NULL) {
        entry->key = lexbor_mraw_alloc(shbst->keys, (key_size + 1));
        if (entry->key == NULL) {
            return NULL;
        }

        for (size_t i = 0; i < key_size; i++) {
            entry->key[i] = lexbor_str_res_map_lowercase[key[i]];
        }

        entry->key[key_size] = '\0';
    }
    else {
        entry->key = NULL;
    }

    entry->value = value;

    return entry;
}

static lexbor_shbst_entry_t *
lexbor_shbst_entry_destroy(lexbor_shbst_t *shbst, lexbor_shbst_entry_t *entry)
{
    if (entry == NULL) {
        return NULL;
    }

    if (entry->key != NULL) {
        lexbor_mraw_free(shbst->keys, entry->key);
    }

    return lexbor_dobject_free(shbst->entries, entry);
}

static lexbor_shbst_entry_t *
lexbor_shbst_search_sbst(lexbor_shbst_t *shbst, lexbor_bst_entry_t *scope,
                         lexbor_shbst_cmp_f cmp_f,
                         const lxb_char_t *key, size_t key_size)
{
    lexbor_bst_entry_t *node;
    lexbor_shbst_entry_t *entry;

    node = lexbor_bst_search(shbst->bst, scope, key_size);

    while (node) {
        entry = (lexbor_shbst_entry_t *) node->value;

        if (cmp_f(entry->key, key, key_size)) {
            return entry;
        }

        node = node->next;
    }

    return NULL;
}

lexbor_shbst_entry_t *
lexbor_shbst_search(lexbor_shbst_t *shbst, const lxb_char_t *key,
                    size_t key_size, bool case_insensitive)
{
    if (key == NULL || key_size == 0) {
        return NULL;
    }

    size_t idx = lexbor_shbst_make_id_m(shbst, key, key_size);

    if (shbst->table[idx] == NULL) {
        return NULL;
    }

    if (case_insensitive) {
        return lexbor_shbst_search_sbst(shbst, shbst->table[idx],
                                        lexbor_str_data_ncasecmp,
                                        key, key_size);
    }

    return lexbor_shbst_search_sbst(shbst, shbst->table[idx],
                                    lexbor_str_data_ncmp, key, key_size);
}

lexbor_shbst_entry_t *
lexbor_shbst_insert(lexbor_shbst_t *shbst,
                    const lxb_char_t *key, size_t key_size, void *value)
{
    size_t idx;
    lexbor_shbst_entry_t *entry;

    if (key == NULL || key_size == 0) {
        return NULL;
    }

    idx = lexbor_shbst_make_id_m(shbst, key, key_size);

    entry = lexbor_shbst_entry_make(shbst, key, key_size, value);
    if (entry == NULL) {
        return NULL;
    }

    if (lexbor_bst_insert(shbst->bst, &shbst->table[idx], key_size, entry)
        == NULL)
    {
        return lexbor_shbst_entry_destroy(shbst, entry);
    }

    return entry;
}

lexbor_shbst_entry_t *
lexbor_shbst_insert_lowercase(lexbor_shbst_t *shbst, const lxb_char_t *key,
                              size_t key_size, void *value)
{
    size_t idx;
    lexbor_shbst_entry_t *entry;

    if (key == NULL || key_size == 0) {
        return NULL;
    }

    idx = lexbor_shbst_make_id_m(shbst, key, key_size);

    entry = lexbor_shbst_entry_make_lowercase(shbst, key, key_size, value);
    if (entry == NULL) {
        return NULL;
    }

    if (lexbor_bst_insert(shbst->bst, &shbst->table[idx], key_size, entry)
        == NULL)
    {
        return lexbor_shbst_entry_destroy(shbst, entry);
    }

    return entry;
}

lexbor_shbst_entry_t *
lexbor_shbst_insert_wo_copy(lexbor_shbst_t *shbst,
                            lxb_char_t *key, size_t key_size, void *value)
{
    size_t idx;
    lexbor_shbst_entry_t *entry;

    if (key == NULL || key_size == 0) {
        return NULL;
    }

    idx = lexbor_shbst_make_id_m(shbst, key, key_size);

    entry = lexbor_shbst_entry_create(shbst);
    if (entry == NULL) {
        return NULL;
    }

    entry->key = key;
    entry->value = value;

    if (lexbor_bst_insert(shbst->bst, &shbst->table[idx], key_size, entry)
        == NULL)
    {
        return lexbor_shbst_entry_destroy(shbst, entry);
    }

    return entry;
}

lexbor_shbst_entry_t *
lexbor_shbst_replace(lexbor_shbst_t *shbst, const lxb_char_t *key,
                     size_t key_size, void *value, bool case_insensitive)
{
    size_t idx;
    lexbor_shbst_entry_t *entry;

    if (key == NULL || key_size == 0) {
        return NULL;
    }

    idx = lexbor_shbst_make_id_m(shbst, key, key_size);

    if (case_insensitive) {
        entry = lexbor_shbst_search_sbst(shbst, shbst->table[idx],
                                         lexbor_str_data_ncasecmp,
                                         key, key_size);
    }
    else {
        entry = lexbor_shbst_search_sbst(shbst, shbst->table[idx],
                                         lexbor_str_data_ncmp, key, key_size);
    }

    if (entry != NULL) {
        entry->value = value;
        return entry;
    }

    entry = lexbor_shbst_entry_make(shbst, key, key_size, value);
    if (entry == NULL) {
        return NULL;
    }

    if (lexbor_bst_insert(shbst->bst, &shbst->table[idx], key_size, entry)
        == NULL)
    {
        return lexbor_shbst_entry_destroy(shbst, entry);
    }

    return entry;
}

/*
 * No inline functions for ABI.
 */
lexbor_mraw_t *
lexbor_shbst_keys_noi(lexbor_shbst_t *shbst)
{
    return lexbor_shbst_keys(shbst);
}
