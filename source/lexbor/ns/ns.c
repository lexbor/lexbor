/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/core/shs.h"

#include "lexbor/ns/ns.h"

#define LXB_NS_RES_DATA
#define LXB_NS_RES_SHS_DATA
#include "lexbor/ns/res.h"


const lxb_ns_data_t *
lxb_ns_data_by_id(lxb_ns_id_t ns_id)
{
    if (ns_id >= LXB_NS__LAST_ENTRY) {
        return NULL;
    }

    return &lxb_ns_res_data[ns_id];
}

const lxb_ns_data_t *
lxb_ns_data_by_name(const lxb_char_t *name, size_t len)
{
    const lexbor_shs_entry_t *entry;

    entry = lexbor_shs_entry_get_static(lxb_ns_res_shs_data, name, len);
    if (entry == NULL) {
        return NULL;
    }

    return (const lxb_ns_data_t *) entry->value;
}

const lxb_char_t *
lxb_ns_name_by_id(lxb_ns_id_t ns_id, size_t *len)
{
    if (ns_id >= LXB_NS__LAST_ENTRY) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len) {
        *len = lxb_ns_res_data[ns_id].name_len;
    }

    return (const lxb_char_t *) lxb_ns_res_data[ns_id].name;
}

const lxb_char_t *
lxb_ns_lower_name_by_id(lxb_ns_id_t ns_id, size_t *len)
{
    if (ns_id >= LXB_NS__LAST_ENTRY) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len != NULL) {
        *len = lxb_ns_res_data[ns_id].name_len;
    }

    return (const lxb_char_t *) lxb_ns_res_data[ns_id].name_lower;
}

const lxb_char_t *
lxb_ns_link_by_id(lxb_ns_id_t ns_id, size_t *len)
{
    if (ns_id >= LXB_NS__LAST_ENTRY) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len) {
        *len = lxb_ns_res_data[ns_id].link_len;
    }

    return (const lxb_char_t *) lxb_ns_res_data[ns_id].link;
}

lxb_ns_id_t
lxb_ns_id_by_name(const lxb_char_t *name, size_t len)
{
    const lexbor_shs_entry_t *entry;

    entry = lexbor_shs_entry_get_static(lxb_ns_res_shs_data, name, len);
    if (entry == NULL) {
        return LXB_NS__UNDEF;
    }

    return ((const lxb_ns_data_t *) entry->value)->ns_id;
}
