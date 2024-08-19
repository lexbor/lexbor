/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_hhea_t *
lxb_font_table_hhea(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_hhea_t *table;
    uint8_t *data;
    uint32_t offset;
    uint16_t vmaj;
    uint16_t vmin;

    table = LXB_FONT_ALLOC(1, lxb_font_table_hhea_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_HHEA];
    data = font_data + offset;

    if (size < (offset + 36)) {
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

    table->ascender = lxb_font_read_16(&data);
    table->descender = lxb_font_read_16(&data);
    table->line_gap = lxb_font_read_16(&data);
    table->advanced_width_max = lxb_font_read_u16(&data);
    table->min_left_side_bearing = lxb_font_read_16(&data);
    table->min_right_side_bearing = lxb_font_read_16(&data);
    table->x_max_extent = lxb_font_read_16(&data);
    table->caret_slope_rise = lxb_font_read_16(&data);
    table->caret_slope_run = lxb_font_read_16(&data);
    table->caret_offset = lxb_font_read_16(&data);

    /* the 4 reserved int16_t, not stored, must be set to 0. */
    for (int i = 0; i < 4; i++) {
        int16_t reserved;

        reserved = lxb_font_read_16(&data);
        if (reserved != 0) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
        }
    }

    table->metric_data_format = lxb_font_read_16(&data);
    table->number_of_h_metrics = lxb_font_read_u16(&data);

    return table;
}
