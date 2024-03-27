/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_os_2_t *
lxb_font_table_os_2(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_os_2_t *table;
    uint8_t *data;
    uint32_t offset;
    uint32_t pos;

    table = LXB_FONT_ALLOC(1, lxb_font_table_os_2_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_OS_2];
    data = font_data + offset;

    pos = offset + 32 + 10 + 16 + 4 + 16;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->version = lxb_font_read_u16(&data);
    if (table->version > 5) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->x_avg_char_width = lxb_font_read_16(&data);
    table->us_weight_class = lxb_font_read_u16(&data);
    table->us_width_class = lxb_font_read_u16(&data);
    table->fs_type = lxb_font_read_u16(&data);
    table->y_subscript_x_size = lxb_font_read_16(&data);
    table->y_subscript_y_size = lxb_font_read_16(&data);
    table->y_subscript_x_offset = lxb_font_read_16(&data);
    table->y_subscript_y_offset = lxb_font_read_16(&data);
    table->y_superscript_x_size = lxb_font_read_16(&data);
    table->y_superscript_y_size = lxb_font_read_16(&data);
    table->y_superscript_x_offset = lxb_font_read_16(&data);
    table->y_superscript_y_offset = lxb_font_read_16(&data);
    table->y_strikeout_size = lxb_font_read_16(&data);
    table->y_strikeout_position = lxb_font_read_16(&data);
    table->s_family_class = lxb_font_read_16(&data);

    memcpy(table->panose, data, 10);
    data += 10;

    table->ul_unicode_range1 = lxb_font_read_u32(&data);
    table->ul_unicode_range2 = lxb_font_read_u32(&data);
    table->ul_unicode_range3 = lxb_font_read_u32(&data);
    table->ul_unicode_range4 = lxb_font_read_u32(&data);

    memcpy(table->ach_vend_id, data, 4);
    data += 4;

    table->fs_selection = lxb_font_read_u16(&data);
    table->us_first_char_index = lxb_font_read_u16(&data);
    table->us_last_char_index = lxb_font_read_u16(&data);
    table->s_typo_ascender = lxb_font_read_16(&data);
    table->s_typo_descender = lxb_font_read_16(&data);
    table->s_typo_line_gap = lxb_font_read_16(&data);
    table->us_win_ascent = lxb_font_read_u16(&data);
    table->us_win_descent = lxb_font_read_u16(&data);

    if (table->version == 0x0001) {
        pos += 8;
        if (size < pos) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
        }

        table->ul_code_page_range1 = lxb_font_read_u32(&data);
        table->ul_code_page_range2 = lxb_font_read_u32(&data);
    }

    if ((table->version >= 0x0002) && (table->version <= 0x0004)) {
        pos += 10;
        if (size < pos) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
        }

        table->sx_height = lxb_font_read_16(&data);
        table->s_cap_height = lxb_font_read_16(&data);
        table->us_default_char = lxb_font_read_u16(&data);
        table->us_break_char = lxb_font_read_u16(&data);
        table->us_max_context = lxb_font_read_u16(&data);
    }

    if (table->version == 0x0005) {
        pos += 4;
        if (size < pos) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
        }

        table->us_lower_optical_point_size = lxb_font_read_u16(&data);
        table->us_upper_optical_point_size = lxb_font_read_u16(&data);
    }

    return table;
}

uint8_t lxb_font_os_2_get_panose(const void *f, lxb_font_os_2_panose_t id)
{
    return ((lxb_font_t *) f)->table_os_2->panose[id];
}
