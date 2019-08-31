/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_SHS_H
#define LEXBOR_SHS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "lexbor/core/base.h"


typedef struct {
    char   *key;
    void   *value;

    size_t key_len;
    size_t next;
}
lexbor_shs_entry_t;


LXB_API const lexbor_shs_entry_t *
lexbor_shs_entry_get_static(const lexbor_shs_entry_t *tree,
                            const lxb_char_t *key, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_SHS_H */





