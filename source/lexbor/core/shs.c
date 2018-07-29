/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/core/shs.h"
#include "lexbor/core/str.h"

#define LEXBOR_STR_RES_MAP_LOWERCASE
#include "lexbor/core/str_res.h"


#define lexbor_shs_make_id_m(key, size, table_size)                            \
    (((((lexbor_str_res_map_lowercase[key[0]]                                  \
     * lexbor_str_res_map_lowercase[key[size - 1]])                            \
     * lexbor_str_res_map_lowercase[key[0]])                                   \
     + size)                                                                   \
     % table_size) + 0x01)


const lexbor_shs_entry_t *
lexbor_shs_entry_get_static(const lexbor_shs_entry_t *root,
                            const lxb_char_t *key, size_t key_len)
{
    const lexbor_shs_entry_t *entry;
    entry = root + lexbor_shs_make_id_m(key, key_len, root->key_len);

    while (entry->key != NULL)
    {
        if (entry->key_len == key_len) {
            if (lexbor_str_data_ncasecmp((const lxb_char_t *) entry->key,
                                         key, key_len))
            {
                return entry;
            }

            entry = &root[entry->next];
        }
        else if (entry->key_len > key_len) {
            return NULL;
        }
        else {
            entry = &root[entry->next];
        }
    }

    return NULL;
}
