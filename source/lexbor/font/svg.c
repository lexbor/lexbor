/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_svg_t *
lxb_font_table_svg(lxb_font_t *mf, uint8_t *font_data, size_t size)
{
    lxb_font_table_svg_t *table;
    uint8_t *data;
    uint32_t offset;
    uint32_t pos;
    uint16_t version;
    uint16_t reserved;
    uint16_t i;

    table = LXB_FONT_ALLOC(1, lxb_font_table_svg_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_SVG];
    data = font_data + offset;

    if (size < (offset + 10)) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    /* version, not stored, must be equal to 0. */
    version = lxb_font_read_u16(&data);
    if (version != 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->offset_to_svg_document_list = lxb_font_read_u32(&data);
    if (table->offset_to_svg_document_list == 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    reserved = lxb_font_read_u32(&data);
    if (reserved != 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    pos = offset + table->offset_to_svg_document_list + 2;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    /* We change offset to go to SVG document */
    data = font_data + offset + table->offset_to_svg_document_list;

    table->num_entries = lxb_font_read_u16(&data);
    if (table->num_entries == 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    pos += table->num_entries * sizeof(svg_document_record_t);
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->document_records = LXB_FONT_ALLOC(table->num_entries, svg_document_record_t);
    if (table->document_records == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    for (i = 0; i < table->num_entries; i++) {
        table->document_records[i].start_glyph_id = lxb_font_read_u16(&data);
        table->document_records[i].end_glyph_id = lxb_font_read_u16(&data);
        table->document_records[i].svg_doc_offset = lxb_font_read_u32(&data);
        table->document_records[i].svg_doc_length = lxb_font_read_u32(&data);
    }

    return table;
}
