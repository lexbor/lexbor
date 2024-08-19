/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_name_t *
lxb_font_table_name(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_name_t *table;
    uint8_t *data;
    uint32_t offset;
    uint32_t pos;
    uint16_t i;

    table = LXB_FONT_ALLOC(1, lxb_font_table_name_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_NAME];
    data = font_data + offset;

    pos = offset + 6;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->format = lxb_font_read_u16(&data);
    if ((table->format != 0) && (table->format != 1)) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->count = lxb_font_read_u16(&data);
    pos += table->count * 12;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->string_offset = lxb_font_read_u16(&data);

    table->name_record = LXB_FONT_ALLOC(table->count, name_record_t);
    if (table->name_record == NULL) {
        table->count = 0;
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    for (i = 0; i < table->count; i++) {
        table->name_record[i].platform_id = lxb_font_read_u16(&data);
        table->name_record[i].encoding_id = lxb_font_read_u16(&data);
        table->name_record[i].language_id = lxb_font_read_u16(&data);
        table->name_record[i].name_id = lxb_font_read_u16(&data);
        table->name_record[i].length = lxb_font_read_u16(&data);
        table->name_record[i].offset = lxb_font_read_u16(&data);
    }

    if (table->format == 1) {
        pos += 2 + table->lang_tag_count * 4;
        if (size < pos) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
        }

        table->lang_tag_count = lxb_font_read_u16(&data);

        table->lang_tag_record = LXB_FONT_ALLOC(table->lang_tag_count,
                                                name_lang_tag_record_t);
        if (table->lang_tag_record == NULL) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
        }

        for (i = 0; i < table->count; i++) {
            table->lang_tag_record[i].length = lxb_font_read_u16(&data);
            table->lang_tag_record[i].offset = lxb_font_read_u16(&data);
        }
    }

    /* FIXME: todo str_data */

    return table;
}
