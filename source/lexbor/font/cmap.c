/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


static lxb_status_t
lxb_font_cmap_format_0(lxb_font_t *mf, cmap_subtable_t *subtable,
                       uint8_t *font_data, size_t size, size_t offset)
{
    cmap_subtable_format_0_t *header;
    uint8_t *data;

    subtable->header = NULL;

    if (size < (offset + 260)) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header = LXB_FONT_ALLOC(1, cmap_subtable_format_0_t);
    if (header == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = font_data + offset;

    header->length = lxb_font_read_u16(&data);
    header->language = lxb_font_read_u16(&data);
    memcpy(header->glyph_id_array, data, 256);

    subtable->header = header;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_font_cmap_format_2(lxb_font_t *mf, cmap_subtable_t *subtable,
                       uint8_t *font_data, size_t size, size_t offset)
{
    cmap_subtable_format_2_t *header;
    uint8_t *data;
    uint16_t keys_data;
    uint32_t sub_headers_count;
    uint8_t i;

    subtable->header = NULL;

    if (size < (offset + 4 + 2 * 256)) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header = LXB_FONT_ALLOC(1, cmap_subtable_format_2_t);
    if (header == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = font_data + offset;

    header->length = lxb_font_read_u16(&data);
    header->language = lxb_font_read_u16(&data);
    for (i = 0; i < 256; i++) {
        header->sub_header_keys[i] = lxb_font_read_u16(&data);
    }

    /*
     * The number of elements of 'subheaders' array is the maximum value
     * found in 'sub_header_keys' field.
     */
    /* FIXME */

    subtable->header = header;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_font_cmap_format_4(lxb_font_t *mf, cmap_subtable_t *subtable,
                       uint8_t *font_data, size_t size, size_t offset)
{
    cmap_subtable_format_4_t *header;
    uint8_t *data;
    uint32_t pos;
    uint16_t reserved;
    uint16_t i;

    subtable->header = NULL;

    pos = offset + 12;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header = LXB_FONT_ALLOC(1, cmap_subtable_format_4_t);
    if (header == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = font_data + offset;

    header->length = lxb_font_read_u16(&data);
    header->language = lxb_font_read_u16(&data);
    header->seg_count_x_2 = lxb_font_read_u16(&data);
    header->search_range = lxb_font_read_u16(&data);
    header->entry_selector = lxb_font_read_u16(&data);
    header->range_shift = lxb_font_read_u16(&data);

    pos += header->seg_count_x_2 * 4 + 2;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header->end_code = LXB_FONT_ALLOC(header->seg_count_x_2, uint8_t);
    if (header->end_code == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < (header->seg_count_x_2 / 2); i++){
        header->end_code[i] = lxb_font_read_u16(&data);
    }

    /* reserved, must be 0 */
    reserved = lxb_font_read_u16(&data);
    if (reserved != 0) {
        return LXB_STATUS_ERROR_UNEXPECTED_DATA;
    }

    header->start_code = LXB_FONT_ALLOC(header->seg_count_x_2, uint8_t);
    if (header->start_code == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < (header->seg_count_x_2 / 2); i++){
        header->start_code[i] = lxb_font_read_u16(&data);
    }

    header->id_delta = LXB_FONT_ALLOC(header->seg_count_x_2, uint8_t);
    if (header->id_delta == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < (header->seg_count_x_2 / 2); i++){
        header->id_delta[i] = lxb_font_read_16(&data);
    }

    header->id_range_offset = LXB_FONT_ALLOC(header->seg_count_x_2, uint8_t);
    if (header->id_range_offset == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < (header->seg_count_x_2 / 2); i++){
        header->id_range_offset[i] = lxb_font_read_u16(&data);
    }

    header->glyph_id_count = ((header->length - (16 + header->seg_count_x_2 * 4)) & 0xffff) / 2;

    header->glyph_id_array = LXB_FONT_ALLOC(header->glyph_id_count, uint8_t);
    if (header->glyph_id_array == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < header->glyph_id_count; i++){
        header->glyph_id_array[i] = lxb_font_read_u16(&data);
    }

    subtable->header = header;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_font_cmap_format_6(lxb_font_t *mf, cmap_subtable_t *subtable,
                       uint8_t *font_data, size_t size, size_t offset)
{
    cmap_subtable_format_6_t *header;
    uint8_t *data;
    uint32_t pos;
    uint16_t i;

    subtable->header = NULL;

    pos = offset + 10;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header = LXB_FONT_ALLOC(1, cmap_subtable_format_6_t);
    if (header == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = font_data + offset;

    header->length = lxb_font_read_u16(&data);
    header->language = lxb_font_read_u16(&data);
    header->first_code = lxb_font_read_u16(&data);
    header->entry_count = lxb_font_read_u16(&data);

    pos +=  header->entry_count;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header->glyph_id_array = LXB_FONT_ALLOC(header->entry_count, uint16_t);
    if (header->glyph_id_array == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < header->entry_count; i++) {
        header->glyph_id_array[i] = lxb_font_read_u16(&data);
    }

    subtable->header = header;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_font_cmap_format_8(lxb_font_t *mf, cmap_subtable_t *subtable,
                       uint8_t *font_data, size_t size, size_t offset)
{
    cmap_subtable_format_8_t *header;
    uint8_t *data;
    uint32_t pos;
    uint32_t i;

    subtable->header = NULL;

    pos = offset + 2 + 4 + 4 + 8192 + 4;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header = LXB_FONT_ALLOC(1, cmap_subtable_format_8_t);
    if (header == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = font_data + offset;

    /* reserved, 2 bytes, ignored */
    lxb_font_read_u16(&data);
    header->length = lxb_font_read_u32(&data);
    header->language = lxb_font_read_u32(&data);
    memcpy(header->is_32, data, 8192);
    data += 8192;
    header->num_groups = lxb_font_read_u32(&data);

    pos +=  header->num_groups;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header->groups = LXB_FONT_ALLOC(header->num_groups, cmap_sequential_map_group_t);
    if (header->groups == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < header->num_groups; i++) {
        header->groups[i].start_char_code = lxb_font_read_u32(&data);
        header->groups[i].end_char_code = lxb_font_read_u32(&data);
        header->groups[i].start_glyph_id = lxb_font_read_u32(&data);
    }

    subtable->header = header;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_font_cmap_format_10(lxb_font_t *mf, cmap_subtable_t *subtable,
                        uint8_t *font_data, size_t size, size_t offset)
{
    cmap_subtable_format_10_t *header;
    uint8_t *data;
    uint32_t pos;
    uint32_t i;

    subtable->header = NULL;

    pos = offset + 2 + 4 + 4 + 4 + 4;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header = LXB_FONT_ALLOC(1, cmap_subtable_format_10_t);
    if (header == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = font_data + offset;

    /* reserved, 2 bytes, ignored */
    lxb_font_read_u16(&data);
    header->length = lxb_font_read_u32(&data);
    header->language = lxb_font_read_u32(&data);
    header->start_char_code = lxb_font_read_u32(&data);
    header->num_chars = lxb_font_read_u32(&data);

    pos +=  header->num_chars;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header->glyphs = LXB_FONT_ALLOC(header->num_chars, uint16_t);
    if (header->glyphs == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < header->num_chars; i++) {
        header->glyphs[i] = lxb_font_read_u16(&data);
    }

    subtable->header = header;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_font_cmap_format_12(lxb_font_t *mf, cmap_subtable_t *subtable,
                        uint8_t *font_data, size_t size, size_t offset)
{
    cmap_subtable_format_12_t *header;
    uint8_t *data;
    uint32_t pos;
    uint32_t i;

    subtable->header = NULL;

    pos = offset + 2 + 4 + 4 + 4;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header = LXB_FONT_ALLOC(1, cmap_subtable_format_12_t);
    if (header == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = font_data + offset;

    /* reserved, 2 bytes, ignored */
    lxb_font_read_u16(&data);
    header->length = lxb_font_read_u32(&data);
    header->language = lxb_font_read_u32(&data);
    header->num_groups = lxb_font_read_u32(&data);

    pos +=  header->num_groups;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header->groups = LXB_FONT_ALLOC(header->num_groups, cmap_sequential_map_group_t);
    if (header->groups == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < header->num_groups; i++) {
        header->groups[i].start_char_code = lxb_font_read_u32(&data);
        header->groups[i].end_char_code = lxb_font_read_u32(&data);
        header->groups[i].start_glyph_id = lxb_font_read_u32(&data);
    }

    subtable->header = header;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_font_cmap_format_13(lxb_font_t *mf, cmap_subtable_t *subtable,
                        uint8_t *font_data, size_t size, size_t offset)
{
    cmap_subtable_format_13_t *header;
    uint8_t *data;
    uint32_t pos;
    uint32_t i;

    subtable->header = NULL;

    pos = offset + 2 + 4 + 4 + 4;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header = LXB_FONT_ALLOC(1, cmap_subtable_format_13_t);
    if (header == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = font_data + offset;

    /* reserved, 2 bytes, ignored */
    lxb_font_read_u16(&data);
    header->length = lxb_font_read_u32(&data);
    header->language = lxb_font_read_u32(&data);
    header->num_groups = lxb_font_read_u32(&data);

    pos +=  header->num_groups;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header->groups = LXB_FONT_ALLOC(header->num_groups, cmap_constant_map_group_t);
    if (header->groups == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < header->num_groups; i++) {
        header->groups[i].start_char_code = lxb_font_read_u32(&data);
        header->groups[i].end_char_code = lxb_font_read_u32(&data);
        header->groups[i].glyph_id = lxb_font_read_u32(&data);
    }

    subtable->header = header;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_font_cmap_format_14(lxb_font_t *mf, cmap_subtable_t *subtable,
                        uint8_t *font_data, size_t size, size_t offset)
{
    cmap_subtable_format_14_t *header;
    uint8_t *data;
    uint8_t *data_uvs;
    uint32_t pos;
    uint32_t i;
    uint32_t j;

    subtable->header = NULL;

    pos = offset + 4 + 4;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header = LXB_FONT_ALLOC(1, cmap_subtable_format_14_t);
    if (header == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = font_data + offset;

    header->length = lxb_font_read_u32(&data);
    header->num_var_selector_records = lxb_font_read_u32(&data);

    pos +=  header->num_var_selector_records;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    header->var_selector = LXB_FONT_ALLOC(header->num_var_selector_records,
                                          cmap_variation_selector_t);
    if (header->var_selector == NULL){
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < header->num_var_selector_records; i++) {
        header->var_selector[i].var_selector = 0;
        memcpy(&header->var_selector[i].var_selector, data, 3);
        data += 3;
        header->var_selector[i].default_uvs_offset = lxb_font_read_u32(&data);
        header->var_selector[i].non_default_uvs_offset = lxb_font_read_u32(&data);
        /* default UVS */
        data_uvs = font_data - 2; /* beginning of the subtable, so going backward
                                of 2 bytes, the length of the format value */
        header->var_selector[i].default_uvs.num_unicode_value_ranges = lxb_font_read_u32(&data_uvs);
        header->var_selector[i].default_uvs.ranges = LXB_FONT_ALLOC(header->var_selector[i].default_uvs.num_unicode_value_ranges,
                                                                    cmap_variation_selector_t);
        if (header->var_selector[i].default_uvs.ranges == NULL){
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        for (j = 0; j < header->var_selector[i].default_uvs.num_unicode_value_ranges; j++) {
            header->var_selector[i].default_uvs.ranges[j].start_unicode_value = 0;
            memcpy(&header->var_selector[i].default_uvs.ranges[j].start_unicode_value, data_uvs, 3);
            data_uvs += 3;
            header->var_selector[i].default_uvs.ranges[j].additional_count = lxb_font_read_u8(&data_uvs);
        }

        /* default UVS */
        data_uvs = font_data - 2; /* beginning of the subtable, so going backward
                                of 2 bytes, the length of the format value */
        header->var_selector[i].non_default_uvs.num_uvs_mappings = lxb_font_read_u32(&data_uvs);
        header->var_selector[i].non_default_uvs.uvs_mappings = LXB_FONT_ALLOC(header->var_selector[i].non_default_uvs.num_uvs_mappings,
                                                                              cmap_uvs_mapping_t);
        if (header->var_selector[i].non_default_uvs.uvs_mappings == NULL){
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        for (j = 0; j < header->var_selector[i].non_default_uvs.num_uvs_mappings; j++) {
            header->var_selector[i].non_default_uvs.uvs_mappings[j].unicode_value = 0;
            memcpy(&header->var_selector[i].non_default_uvs.uvs_mappings[j].unicode_value, data_uvs, 3);
            data_uvs += 3;
            header->var_selector[i].non_default_uvs.uvs_mappings[j].glyph_id = lxb_font_read_u8(&data_uvs);
        }
    }

    subtable->header = header;

    return LXB_STATUS_OK;
}

lxb_font_table_cmap_t *
lxb_font_table_cmap(lxb_font_t *mf, uint8_t *font_data, size_t size)
{
    lxb_font_table_cmap_t *table;
    uint8_t *data;
    uint32_t offset;
    uint32_t pos;
    uint32_t i;
    uint32_t offset_subtable;
    uint16_t version;

    table = LXB_FONT_ALLOC(1, lxb_font_table_cmap_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_CMAP];
    data = font_data + offset;

    pos = offset + 4;
    if (size < pos) {
        goto failed;
    }

    version = lxb_font_read_u16(&data);
    if (version != 0) {
        goto failed;
    }

    table->num_tables = lxb_font_read_u16(&data);
    pos += table->num_tables * (2 + 2 + 4 + 2 /* format */);
    if (size < pos) {
        goto failed;
    }

    table->encoding_records = LXB_FONT_ALLOC(table->num_tables,
                                             cmap_encoding_record_t);
    if (table->encoding_records == NULL) {
        goto failed;
    }

    table->subtables = LXB_FONT_ALLOC(table->num_tables, cmap_subtable_t);
    if (table->subtables == NULL) {
        goto failed;
    }

    for (i = 0; i < table->num_tables; i++) {
        table->encoding_records[i].platform_id = lxb_font_read_u16(&data);
        table->encoding_records[i].encoding_id = lxb_font_read_u16(&data);
        table->encoding_records[i].offset = lxb_font_read_u32(&data);
    }

    for (i = 0; i < table->num_tables; i++) {
        offset_subtable = offset + table->encoding_records[i].offset;

        if (size < offset_subtable) {
            goto failed;
        }

        data = font_data + offset_subtable;
        table->subtables[i].format = lxb_font_read_u16(&data);
        /* Subtable format has just been read, so increase ofsset by 2. */
        offset_subtable += 2;

        switch (table->subtables[i].format) {
            case 0:
                mf->status = lxb_font_cmap_format_0(mf, table->subtables + i,
                                                    font_data, size, offset_subtable);
                break;
            case 2:
                mf->status = lxb_font_cmap_format_2(mf, table->subtables + i,
                                                    font_data, size, offset_subtable);
                break;
            case 4:
                mf->status = lxb_font_cmap_format_4(mf, table->subtables + i,
                                                    font_data, size, offset_subtable);
                break;
            case 6:
                mf->status = lxb_font_cmap_format_6(mf, table->subtables + i,
                                                    font_data, size, offset_subtable);
                break;
            case 8:
                mf->status = lxb_font_cmap_format_8(mf, table->subtables + i,
                                                    font_data, size, offset_subtable);
                break;
            case 10:
                mf->status = lxb_font_cmap_format_10(mf, table->subtables + i,
                                                     font_data, size, offset_subtable);
                break;
            case 12:
                mf->status = lxb_font_cmap_format_12(mf, table->subtables + i,
                                                     font_data, size, offset_subtable);
                break;
            case 13:
                mf->status = lxb_font_cmap_format_13(mf, table->subtables + i,
                                                     font_data, size, offset_subtable);
                break;
            case 14:
                mf->status = lxb_font_cmap_format_14(mf, table->subtables + i,
                                                     font_data, size, offset_subtable);
                break;
            default:
                return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
        }

        if (mf->status != LXB_STATUS_OK) {
            return lexbor_mraw_free(mf->mraw, table);
        }
    }

    return table;

  failed:

    lexbor_mraw_free(mf->mraw, table);

    return lxb_font_failed(mf, LXB_STATUS_ERROR);
}
