/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_maxp_t *
lxb_font_table_maxp(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_maxp_t *table;
    uint8_t *data;
    uint32_t offset;
    int32_t version;

    table = LXB_FONT_ALLOC(1, lxb_font_table_maxp_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_MAXP];
    data = font_data + offset;

    if (size < (offset + 32)) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    /*
     * Version not stored.
     * - If font is Truetype, version must be equal to 0x00005000,
     * - Otherwise (OpenType CFF), version must be equal to 0x00010000.
     */
    version = lxb_font_read_32(&data);
    if ((!mf->is_truetype && (version != 0x00005000)) ||
        (mf->is_truetype && (version != 0x00010000))) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->num_glyphs = lxb_font_read_u16(&data);

    if (mf->is_truetype == 0) {
        table->max_points = lxb_font_read_u16(&data);
        table->max_contours = lxb_font_read_u16(&data);
        table->max_composite_points = lxb_font_read_u16(&data);
        table->max_composite_contours = lxb_font_read_u16(&data);
        table->max_zones = lxb_font_read_u16(&data);
        table->max_twilight_points = lxb_font_read_u16(&data);
        table->max_storage = lxb_font_read_u16(&data);
        table->max_function_defs = lxb_font_read_u16(&data);
        table->max_instruction_defs = lxb_font_read_u16(&data);
        table->max_stack_elements = lxb_font_read_u16(&data);
        table->max_size_of_instructions = lxb_font_read_u16(&data);
        table->max_component_elements = lxb_font_read_u16(&data);
        table->max_component_depth = lxb_font_read_u16(&data);
    }

    return table;
}
