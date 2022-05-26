/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/css/css.h"
#include "lexbor/css/value/res.h"


lxb_css_value_type_t
lxb_css_value_by_name(const lxb_char_t *name, size_t length)
{
    const lexbor_shs_entry_t *entry;

    entry = lexbor_shs_entry_get_lower_static(lxb_css_value_shs, name, length);
    if (entry == NULL) {
        return LXB_CSS_VALUE__UNDEF;
    }

    return (lxb_css_value_type_t) (uintptr_t) entry->value;
}

lxb_status_t
lxb_css_value_serialize(lxb_css_value_type_t type,
                        lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_data_t *data;

    if (type >= LXB_CSS_VALUE__LAST_ENTRY) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    data = &lxb_css_value_data[type];

    return cb(data->name, data->length, ctx);
}
