/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_sbix_t *
lxb_font_table_sbix(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_sbix_t *table;
    uint8_t *data;
    uint8_t *data_strike;
    uint8_t *data_glyph;
    uint32_t *data_offset;
    uint32_t offset;
    uint32_t data_length;
    uint32_t pos;
    uint32_t i;
    uint32_t j;
    uint16_t num_glyphs;
    uint16_t version;

    table = LXB_FONT_ALLOC(1, lxb_font_table_sbix_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_SBIX];
    data = font_data + offset;

    num_glyphs = mf->table_maxp->num_glyphs;
    if (num_glyphs == 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    pos = offset + 8;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    version = lxb_font_read_u16(&data);
    if (version != 1) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->flags = lxb_font_read_u16(&data);
    table->num_strikes = lxb_font_read_u32(&data);

    pos += offset + 4 * table->num_strikes;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->strikes = LXB_FONT_ALLOC(table->num_strikes, sbix_strike_t);
    if (table->strikes == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    data_offset = malloc((num_glyphs + 1) * sizeof(uint32_t));
    if (data_offset == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    /* FIXME: update pos ! */
    for (i = 0; i < table->num_strikes; i++) {
        data_strike = font_data + offset;
        data_strike += lxb_font_read_u32(&data);
        table->strikes[i].ppem = lxb_font_read_u16(&data_strike);
        table->strikes[i].ppi = lxb_font_read_u16(&data_strike);
        for (j = 0; j <= num_glyphs; j++) {
            data_offset[j] = lxb_font_read_u32(&data_strike);
        }

        table->strikes[i].glyphs = LXB_FONT_ALLOC(num_glyphs, sbix_glyph_data_t);
        if (table->strikes[i].glyphs == NULL) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
        }

        data_glyph = (uint8_t *) (table->strikes + i);

        for (j = 0; j < num_glyphs; j++) {
            data_glyph += data_offset[j];
            table->strikes[i].glyphs[j].origin_offset_x = lxb_font_read_16(&data_glyph);
            table->strikes[i].glyphs[j].origin_offset_y = lxb_font_read_16(&data_glyph);
            memcpy(table->strikes[i].glyphs[j].data, data_glyph, 4);
            data_glyph += 4;

            data_length = data_offset[j + 1] - data_offset[j] - 8;
            table->strikes[i].glyphs[j].data = LXB_FONT_ALLOC(data_length, uint8_t);
            if (table->strikes[i].glyphs[j].data == NULL) {
                return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
            }
            memcpy(table->strikes[i].glyphs[j].data, data_glyph, data_length);
        }
    }

    free(data_offset);

    return table;
}
