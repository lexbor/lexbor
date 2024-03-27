/*
 * Copyright (C) 2020 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_mvar_t *
lxb_font_table_mvar(lxb_font_t *mf, uint8_t *font_data, size_t size)
{
    lxb_font_table_mvar_t *table;
    uint8_t *data;
    uint32_t offset;
    uint16_t offset_item;
    uint32_t pos;
    uint16_t vmaj;
    uint16_t vmin;
    uint16_t reserved;
    uint16_t i;

    table = LXB_FONT_ALLOC(1, lxb_font_table_mvar_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_MVAR];
    data = font_data + offset;

    pos = offset + 12;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    /* major version, not stored, must be equal to 1. */
    vmaj = lxb_font_read_u16(&data);
    if (vmaj != 1) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    /* minor version, not stored, must be equal to 0. */
    vmin = lxb_font_read_u16(&data);
    if (vmin != 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    /* reserved, not stored, must be equal to 0. */
    reserved = lxb_font_read_u16(&data);
    if (reserved != 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->value_record_size = lxb_font_read_u16(&data);
    table->value_record_count = lxb_font_read_u16(&data);
    offset_item = lxb_font_read_u16(&data);

    if (table->value_record_count > 0) {
        mvar_value_record_t *iter;

        pos += table->value_record_count * 8;
        if (size < pos) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
        }

        table->value_records = LXB_FONT_ALLOC(1, mvar_value_record_t);
        if (table->value_records == NULL) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
        }

        iter = table->value_records;
        for (i = 0; i < table->value_record_count; i++, iter++) {
            memcpy(&iter->value_tag, data, 4);
            data += 4;
            iter->delta_set_outer_index = lxb_font_read_u16(&data);
            iter->delta_set_inner_index = lxb_font_read_u16(&data);
        }
    }

    if (offset_item > 0) {
        item_variation_store_t ivs;
        lxb_status_t st;

        offset += offset_item;
        data = font_data + offset;
        st = lxb_font_item_variation_store_fill(mf, offset, data, size, &ivs);
        if (st != LXB_STATUS_OK) {
            return lxb_font_failed(mf, st);
        }
        table->item_variation_store = ivs;
    }

    return table;
}
