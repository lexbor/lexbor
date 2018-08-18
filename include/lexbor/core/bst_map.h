/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_BST_MAP_H
#define LEXBOR_BST_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/bst.h>
#include <lexbor/core/str.h>
#include <lexbor/core/mraw.h>
#include <lexbor/core/dobject.h>


struct {
    lexbor_str_t str;
    void         *value;
}
typedef lexbor_bst_map_entry_t;

struct {
    lexbor_bst_t     *bst;
    lexbor_mraw_t    *mraw;
    lexbor_dobject_t *entries;

}
typedef lexbor_bst_map_t;


lexbor_bst_map_t *
lexbor_bst_map_create(void);

lxb_status_t
lexbor_bst_map_init(lexbor_bst_map_t *bst_map, size_t size);

void
lexbor_bst_map_clean(lexbor_bst_map_t *bst_map);

lexbor_bst_map_t *
lexbor_bst_map_destroy(lexbor_bst_map_t *bst_map, bool self_destroy);


lexbor_bst_map_entry_t *
lexbor_bst_map_search(lexbor_bst_map_t *bst_map, lexbor_bst_entry_t *scope,
                      const lxb_char_t *key, size_t key_len);

lexbor_bst_map_entry_t *
lexbor_bst_map_insert(lexbor_bst_map_t *bst_map, lexbor_bst_entry_t **scope,
                      const lxb_char_t *key, size_t key_len, void *value);

lexbor_bst_map_entry_t *
lexbor_bst_map_insert_not_exists(lexbor_bst_map_t *bst_map,
                                 lexbor_bst_entry_t **scope,
                                 const lxb_char_t *key, size_t key_len);

void *
lexbor_bst_map_remove(lexbor_bst_map_t *bst_map, lexbor_bst_entry_t **scope,
                      const lxb_char_t *key, size_t key_len);


/*
 * Inline functions
 */
lxb_inline lexbor_mraw_t *
lexbor_bst_map_mraw(lexbor_bst_map_t *bst_map)
{
    return bst_map->mraw;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_BST_MAP_H */

