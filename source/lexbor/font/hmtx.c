/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_hmtx_t *
lxb_font_table_hmtx(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_hmtx_t *table;
    uint8_t *data;
    uint32_t offset;
    uint16_t num_metrics;
    uint16_t num_glyphs;
    uint16_t num_lsb;

    table = LXB_FONT_ALLOC(1, lxb_font_table_hmtx_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_HMTX];
    data = font_data + offset;

    num_metrics = mf->table_hhea->number_of_h_metrics;
    num_glyphs = mf->table_maxp->num_glyphs;
    if ((num_metrics == 0) && (num_glyphs == 0)) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    num_lsb = (num_metrics < num_glyphs) ? (num_glyphs - num_metrics) : 0;

    if (size < (offset + num_metrics * 4 + num_lsb * 2)) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->h_metrics = LXB_FONT_ALLOC(num_metrics, hmtx_long_hor_metric_t);
    if (table->h_metrics == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    for (uint16_t i = 0; i < num_metrics; i++) {
        table->h_metrics[i].advance_width = lxb_font_read_u16(&data);
        table->h_metrics[i].lsb = lxb_font_read_16(&data);
    }

    if (num_lsb > 0) {
      table->left_side_bearings = LXB_FONT_ALLOC(num_lsb, int16_t);
        if (table->left_side_bearings == NULL) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
        }

        for (uint16_t i = 0; i < num_lsb; i++) {
            table->left_side_bearings[i] = lxb_font_read_16(&data);
        }
    }

    return table;
}
