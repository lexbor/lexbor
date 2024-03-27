/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_loca_t *
lxb_font_table_loca(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_loca_t *table;
    uint8_t *data;
    uint32_t offset;
    uint32_t pos;
    uint32_t offset_size;
    int16_t index_to_loc;
    uint16_t num_glyphs;
    uint16_t i;

    num_glyphs = mf->table_maxp->num_glyphs;
    if (num_glyphs == 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    /* Specification of loca table says that there is num_glyph + 1 offsets. */
    if (num_glyphs == 65534) {
      return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    num_glyphs++;

    table = LXB_FONT_ALLOC(1, lxb_font_table_loca_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_LOCA];
    data = font_data + offset;

    index_to_loc = mf->table_head->index_to_loc_format;
    offset_size = (index_to_loc == 1) ? (num_glyphs * 4) : (num_glyphs * 2);
    pos = offset + offset_size;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->offsets = LXB_FONT_ALLOC(offset_size, uint8_t);
    if (table->offsets == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    if (index_to_loc == 1) {
        for (i = 0; i < num_glyphs; i++) {
            table->offsets[i] = lxb_font_read_u32(&data);
        }
    }
    else {
        for (i = 0; i < num_glyphs; i++) {
            table->offsets[i] = lxb_font_read_u16(&data) * 2;
        }
    }

    return table;
}

uint32_t
lxb_font_loca_get_offset(lxb_font_t *mf, uint16_t glyph_index)
{
    if(glyph_index >= mf->table_maxp->num_glyphs)
        return mf->table_loca->offsets[0];

    return mf->table_loca->offsets[glyph_index];
}
