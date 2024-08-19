/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_head_t *
lxb_font_table_head(lxb_font_t *mf, uint8_t *font_data, size_t size)
{
    lxb_font_table_head_t *table;
    uint8_t *data;
    uint32_t offset;
    uint16_t vmaj;
    uint16_t vmin;
    uint32_t magic;

    table = LXB_FONT_ALLOC(1, lxb_font_table_head_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_HEAD];
    data = font_data + offset;

    if (size < (offset + 54)) {
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

    table->font_revision = lxb_font_read_32(&data);
    /* FIXME: calculate checksum ? */
    table->check_sum_adjustment = lxb_font_read_u32(&data);

    magic = lxb_font_read_u32(&data);
    if (magic != 0x5f0f3cf5) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->flags = lxb_font_read_u16(&data);
    table->units_per_em = lxb_font_read_u16(&data);
    /* created and modified time font not stored. */
    lxb_font_read_32(&data);
    lxb_font_read_32(&data);
    lxb_font_read_32(&data);
    lxb_font_read_32(&data);
    table->xmin = lxb_font_read_16(&data);
    table->ymin = lxb_font_read_16(&data);
    table->xmax = lxb_font_read_16(&data);
    table->ymax = lxb_font_read_16(&data);
    table->mac_style = lxb_font_read_u16(&data);
    table->lowest_rec_ppem = lxb_font_read_u16(&data);
    table->font_direction_hint = lxb_font_read_16(&data);
    table->index_to_loc_format = lxb_font_read_16(&data);
    table->glyph_data_format = lxb_font_read_16(&data);

    return table;
}
