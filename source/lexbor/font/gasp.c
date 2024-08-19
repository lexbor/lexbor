/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_gasp_t *
lxb_font_table_gasp(lxb_font_t *mf, uint8_t *font_data, size_t size)
{
    lxb_font_table_gasp_t *table;
    uint8_t *data;
    uint32_t offset;
    uint32_t pos;
    uint16_t version;
    uint16_t i;

    table = LXB_FONT_ALLOC(1, lxb_font_table_gasp_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_GASP];
    data = font_data + offset;

    pos = offset + 4;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    /* version, not stored, must be equal to 1. */
    version = lxb_font_read_u16(&data);
    if (version != 1) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->num_ranges = lxb_font_read_u16(&data);

    pos += table->num_ranges * sizeof(gasp_range_t);
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->gasp_ranges = LXB_FONT_ALLOC(table->num_ranges, gasp_range_t);
    if (table->gasp_ranges == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    for (i = 0; i < table->num_ranges; i++) {
        table->gasp_ranges[i].range_max_ppem = lxb_font_read_u16(&data);
        table->gasp_ranges[i].range_gasp_behavior = lxb_font_read_u16(&data);
    }

    return table;
}
