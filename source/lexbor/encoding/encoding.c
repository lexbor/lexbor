/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/encoding/encoding.h"


const lxb_encoding_data_t *
lxb_encoding_data_by_pre_name(const lxb_char_t *name, size_t length)
{
    const lxb_char_t *end;
    const lexbor_shs_entry_t *entry;

    if (length == 0) {
        return NULL;
    }

    end = name + length;

    /* Remove any leading */
    do {
        switch (*name) {
            case 0x09: case 0x0A: case 0x0C: case 0x0D: case 0x20:
                name++;
                continue;
        }

        break;
    }
    while (name < end);

    /* Remove any trailing */
    while (name < end) {
        switch (*(end - 1)) {
            case 0x09: case 0x0A: case 0x0C: case 0x0D: case 0x20:
                end--;
                continue;
        }

        break;
    }

    if (name == end) {
        return NULL;
    }

    entry = lexbor_shs_entry_get_static(lxb_encoding_res_shs_entities,
                                        name, (end - name));
    if (entry == NULL) {
        return NULL;
    }

    return entry->value;
}
