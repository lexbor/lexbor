/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_post_t *
lxb_font_table_post(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_post_t *table;
    uint8_t *data;
    uint32_t offset;
    uint32_t pos;
    uint16_t i;

    table = LXB_FONT_ALLOC(1, lxb_font_table_post_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_POST];
    data = font_data + offset;

    pos = offset + 8 + 4 + 20;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    /* version must be 0x00010000, 0x00020000, 0x00025000 or 0x00030000. */
    table->version = lxb_font_read_32(&data);
    if ((table->version != 0x00010000) &&
        (table->version != 0x00020000) &&
        (table->version != 0x00025000) &&
        (table->version != 0x00030000)) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->italic_angle = lxb_font_read_32(&data);
    table->underline_position = lxb_font_read_16(&data);
    table->underline_thickness = lxb_font_read_16(&data);
    table->is_fixed_pitch = lxb_font_read_u32(&data);
    table->min_mem_type42 = lxb_font_read_u32(&data);
    table->max_mem_type42 = lxb_font_read_u32(&data);
    table->min_mem_type1 = lxb_font_read_u32(&data);
    table->max_mem_type1 = lxb_font_read_u32(&data);

    if ((table->version == 0x00020000) || (table->version == 0x00025000)) {
        pos += 2;
        if (size < pos) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
        }

        table->num_glyphs = lxb_font_read_u16(&data);
        if (table->num_glyphs != mf->table_maxp->num_glyphs) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
        }

        if (table->version == 0x00020000) {
            pos += table->num_glyphs * 2;
            if (size < pos) {
                return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
            }

            table->glyph_name_index = LXB_FONT_ALLOC(table->num_glyphs, uint16_t);
            if (table->glyph_name_index == NULL) {
                return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
            }

            for (i = 0; i < table->num_glyphs; i++) {
                table->glyph_name_index[i] = lxb_font_read_u16(&data);
            }

            /* FIXME: todo... */
            /* table->names = ..... */
        }
        else {
            pos += table->num_glyphs * 1;
            if (size < pos) {
                return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
            }

            table->offset = LXB_FONT_ALLOC(table->num_glyphs, uint8_t);
            if (table->offset == NULL) {
                return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
            }

            for (i = 0; i < table->num_glyphs; i++) {
                table->offset[i] = lxb_font_read_u8(&data);
            }
        }
    }

    return table;
}
