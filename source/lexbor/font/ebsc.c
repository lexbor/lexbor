/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"

static s_bit_line_metrics
lxb_font_table_ebsc_metrics_load(uint8_t *data)
{
    s_bit_line_metrics metrics;

    metrics.ascender = lxb_font_read_8(&data);
    metrics.descender = lxb_font_read_8(&data);
    metrics.width_max = lxb_font_read_u8(&data);
    metrics.caret_slope_numerator = lxb_font_read_8(&data);
    metrics.caret_slope_denominator = lxb_font_read_8(&data);
    metrics.caret_offset = lxb_font_read_8(&data);
    metrics.min_origin_sb = lxb_font_read_8(&data);
    metrics.min_advance_sb = lxb_font_read_8(&data);
    metrics.max_before_bl = lxb_font_read_8(&data);
    metrics.min_after_bl = lxb_font_read_8(&data);
    metrics.pad1 = lxb_font_read_8(&data);
    metrics.pad2 = lxb_font_read_8(&data);

    return metrics;
}

lxb_font_table_ebsc_t *
lxb_font_table_ebsc(lxb_font_t *mf, uint8_t *font_data, size_t size)
{
    lxb_font_table_ebsc_t *table;
    uint8_t *data;
    uint32_t offset;
    uint32_t pos;
    uint16_t vmaj;
    uint16_t vmin;
    uint32_t i;

    table = LXB_FONT_ALLOC(1, lxb_font_table_ebsc_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_EBSC];
    data = font_data + offset;

    pos = offset + 8;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    /* major version, not stored, must be equal to 2. */
    vmaj = lxb_font_read_u16(&data);
    if (vmaj != 2) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    /* minor version, not stored, must be equal to 0. */
    vmin = lxb_font_read_u16(&data);
    if (vmin != 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->num_sizes = lxb_font_read_u32(&data);

    pos +=  table->num_sizes * ( + 4);
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->bitmap_scales = LXB_FONT_ALLOC(table->num_sizes, bitmap_scale_t);
    if (table->bitmap_scales == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    for (i = 0; i < table->num_sizes; i++) {
        table->bitmap_scales[i].hori = lxb_font_table_ebsc_metrics_load(data);
        table->bitmap_scales[i].vert = lxb_font_table_ebsc_metrics_load(data);
        table->bitmap_scales[i].ppem_x = lxb_font_read_u8(&data);
        table->bitmap_scales[i].ppem_y = lxb_font_read_u8(&data);
        table->bitmap_scales[i].substitute_ppem_x = lxb_font_read_u8(&data);
        table->bitmap_scales[i].substitute_ppem_y = lxb_font_read_u8(&data);
    }

    return table;
}
