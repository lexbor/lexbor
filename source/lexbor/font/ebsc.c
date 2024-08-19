/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"
#include "lexbor/font/font_common.h"

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

    pos +=  table->num_sizes * (12 * 2 + 4);
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->bitmap_scales = LXB_FONT_ALLOC(table->num_sizes,
                                          ebsc_bitmap_scale_t);
    if (table->bitmap_scales == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    for (i = 0; i < table->num_sizes; i++) {
        lxb_font_s_bit_line_metrics_fill(data, &table->bitmap_scales[i].hori);
        lxb_font_s_bit_line_metrics_fill(data, &table->bitmap_scales[i].vert);
        table->bitmap_scales[i].ppem_x = lxb_font_read_u8(&data);
        table->bitmap_scales[i].ppem_y = lxb_font_read_u8(&data);
        table->bitmap_scales[i].substitute_ppem_x = lxb_font_read_u8(&data);
        table->bitmap_scales[i].substitute_ppem_y = lxb_font_read_u8(&data);
    }

    return table;
}
