/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_ENCODING_ENCODING_H
#define LEXBOR_ENCODING_ENCODING_H

#ifdef __cplusplus
extern "C" {
#endif


#include "lexbor/encoding/base.h"
#include "lexbor/encoding/res.h"

#include "lexbor/core/shs.h"


/*
 * Before searching will be removed any leading and trailing
 * ASCII whitespace in name.
 */
LXB_API const lxb_encoding_data_t *
lxb_encoding_data_by_pre_name(const lxb_char_t *name, size_t length);


/*
 * Inline functions
 */
lxb_inline const lxb_encoding_data_t *
lxb_encoding_data_by_name(const lxb_char_t *name, size_t length)
{
    const lexbor_shs_entry_t *entry;

    if (length == 0) {
        return NULL;
    }

    entry = lexbor_shs_entry_get_static(lxb_encoding_res_shs_entities,
                                        name, length);
    if (entry == NULL) {
        return NULL;
    }

    return entry->value;
}

lxb_inline const lxb_encoding_data_t *
lxb_encoding_data(lxb_encoding_t encoding)
{
    if (encoding >= LXB_ENCODING_LAST_ENTRY) {
        return NULL;
    }

    return &lxb_encoding_res_map[encoding];
}

lxb_inline lxb_encoding_encode_f
lxb_encoding_encode_function(lxb_encoding_t encoding)
{
    if (encoding >= LXB_ENCODING_LAST_ENTRY) {
        return NULL;
    }

    return lxb_encoding_res_map[encoding].encode;
}

lxb_inline lxb_encoding_decode_f
lxb_encoding_decode_function(lxb_encoding_t encoding)
{
    if (encoding >= LXB_ENCODING_LAST_ENTRY) {
        return NULL;
    }

    return lxb_encoding_res_map[encoding].decode;
}

/*
 * No inline functions for ABI.
 */
const lxb_encoding_data_t *
lxb_encoding_data_by_name_noi(const lxb_char_t *name, size_t length);

const lxb_encoding_data_t *
lxb_encoding_data_noi(lxb_encoding_t encoding);

lxb_encoding_encode_f
lxb_encoding_encode_function_noi(lxb_encoding_t encoding);

lxb_encoding_decode_f
lxb_encoding_decode_function_noi(lxb_encoding_t encoding);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_ENCODING_ENCODING_H */
