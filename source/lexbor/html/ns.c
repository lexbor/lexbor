/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/core/shs.h"

#include "lexbor/html/ns.h"

#define LXB_HTML_NS_RES_DATA
#define LXB_HTML_NS_RES_SHS_DATA
#include "lexbor/html/ns_res.h"


const lxb_html_ns_data_t *
lxb_html_ns_data_by_id(lxb_html_ns_id_t ns_id)
{
    if (ns_id >= LXB_HTML_NS__LAST_ENTRY) {
        return NULL;
    }

    return &lxb_html_ns_res_data[ns_id];
}

const lxb_html_ns_data_t *
lxb_html_ns_data_by_name(const lxb_char_t *name, size_t len)
{
    const lexbor_shs_entry_t *entry;

    entry = lexbor_shs_entry_get_static(lxb_html_ns_res_shs_data, name, len);
    if (entry == NULL) {
        return NULL;
    }

    return (const lxb_html_ns_data_t *) entry->value;
}

const lxb_char_t *
lxb_html_ns_name_by_id(lxb_html_ns_id_t ns_id, size_t *len)
{
    if (ns_id >= LXB_HTML_NS__LAST_ENTRY) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len) {
        *len = lxb_html_ns_res_data[ns_id].name_len;
    }

    return (const lxb_char_t *) lxb_html_ns_res_data[ns_id].name;
}

const lxb_char_t *
lxb_html_ns_lower_name_by_id(lxb_html_ns_id_t ns_id, size_t *len)
{
    if (ns_id >= LXB_HTML_NS__LAST_ENTRY) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len != NULL) {
        *len = lxb_html_ns_res_data[ns_id].name_len;
    }

    return (const lxb_char_t *) lxb_html_ns_res_data[ns_id].name_lower;
}

const lxb_char_t *
lxb_html_ns_link_by_id(lxb_html_ns_id_t ns_id, size_t *len)
{
    if (ns_id >= LXB_HTML_NS__LAST_ENTRY) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len) {
        *len = lxb_html_ns_res_data[ns_id].link_len;
    }

    return (const lxb_char_t *) lxb_html_ns_res_data[ns_id].link;
}

lxb_html_ns_id_t
lxb_html_ns_id_by_name(const lxb_char_t *name, size_t len)
{
    const lexbor_shs_entry_t *entry;

    entry = lexbor_shs_entry_get_static(lxb_html_ns_res_shs_data, name, len);
    if (entry == NULL) {
        return LXB_HTML_NS__UNDEF;
    }

    return ((const lxb_html_ns_data_t *) entry->value)->ns_id;
}
