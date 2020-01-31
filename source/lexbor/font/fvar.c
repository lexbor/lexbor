/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


static lxb_status_t
instance_record_fill(lxb_font_t *mf,
                     lxb_font_table_fvar_t *table,
                     uint8_t *data,
                     size_t size,
                     fvar_instance_t *ir)
{
    uint16_t i;

    ir->subfamily_name_id = lxb_font_read_32(&data);
    ir->flags = lxb_font_read_32(&data);

    ir->coordinates = LXB_FONT_ALLOC(table->axis_count, int32_t);
    if (ir->coordinates == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < mf->table_fvar->axis_count; i++) {
        ir->coordinates[i] = lxb_font_read_32(&data);
    }

    if (table->instance_size == (table->axis_count * 4 + 6)) {
        ir->postscript_name_id = lxb_font_read_u16(&data);
    }

    return LXB_STATUS_OK;
}

static void
variation_axis_record_fill(lxb_font_t *mf,
                           uint8_t *data,
                           size_t size,
                           fvar_variation_axis_t *var)
{
    memcpy(var->axis_tag, data, 4);
    data += 4;
    var->min_value = lxb_font_read_32(&data);
    var->default_value = lxb_font_read_32(&data);
    var->max_value = lxb_font_read_32(&data);
    var->flags = lxb_font_read_u16(&data);
    var->axis_name_id = lxb_font_read_u16(&data);
}

lxb_font_table_fvar_t *
lxb_font_table_fvar(lxb_font_t *mf, uint8_t *font_data, size_t size)
{
    lxb_font_table_fvar_t *table;
    fvar_variation_axis_t *iter_var;
    fvar_instance_t *iter_ins;
    uint8_t *data;
    uint32_t offset;
    uint32_t offset_array;
    uint32_t pos;
    lxb_status_t status;
    uint16_t vmaj;
    uint16_t vmin;
    uint16_t reserved;
    uint16_t i;

    table = LXB_FONT_ALLOC(1, lxb_font_table_fvar_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_FVAR];
    data = font_data + offset;

    if (size < (offset + 16)) {
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

    offset_array = lxb_font_read_u16(&data);
    reserved = lxb_font_read_u16(&data);
    if (reserved != 2) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    table->axis_count = lxb_font_read_u16(&data);
    table->axis_size = lxb_font_read_u16(&data);
    if (table->axis_size != 20) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }
    table->instance_count = lxb_font_read_u16(&data);
    table->instance_size = lxb_font_read_u16(&data);

    /* Variation axis record. */
    offset += offset_array;
    data = font_data + offset;
    pos = offset + 20;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->variation_axis = LXB_FONT_ALLOC(table->axis_count,
                                           fvar_variation_axis_t);
    if (table->variation_axis == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    iter_var = table->variation_axis;
    for (i = 0; i < table->axis_count; i++, iter_var++) {
        variation_axis_record_fill(mf, data, size, iter_var);
    }

    /* Instance record. */
    pos += table->instance_size * table->instance_count;
    if (size < pos) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->instances = LXB_FONT_ALLOC(table->instance_count,
                                      fvar_instance_t);
    if (table->instances == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    iter_ins = table->instances;
    for (i = 0; i < table->instance_count; i++, iter_ins++) {
        status = instance_record_fill(mf, table, data, size, iter_ins);
        if (status != LXB_STATUS_OK) {
            return lxb_font_failed(mf, status);
        }
    }

    return table;
}
