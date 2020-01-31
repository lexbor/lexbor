/*
 * Copyright (C) 2020 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"
#include "lexbor/font/font_common.h"
#include "lexbor/font/font.h"


void
lxb_font_s_bit_line_metrics_fill(uint8_t *data,
                                 s_bit_line_metrics_t *metrics)
{
    metrics->ascender = lxb_font_read_8(&data);
    metrics->descender = lxb_font_read_8(&data);
    metrics->width_max = lxb_font_read_u8(&data);
    metrics->caret_slope_numerator = lxb_font_read_8(&data);
    metrics->caret_slope_denominator = lxb_font_read_8(&data);
    metrics->caret_offset = lxb_font_read_8(&data);
    metrics->min_origin_sb = lxb_font_read_8(&data);
    metrics->min_advance_sb = lxb_font_read_8(&data);
    metrics->max_before_bl = lxb_font_read_8(&data);
    metrics->min_after_bl = lxb_font_read_8(&data);
    metrics->pad1 = lxb_font_read_8(&data);
    metrics->pad2 = lxb_font_read_8(&data);
}

/* fvar table must be loaded first */

static lxb_status_t
variation_region_list_fill(lxb_font_t *mf,
                           uint32_t pos,
                           uint8_t *data,
                           size_t size,
                           variation_region_list_t *vrl)
{
    variation_region_t *iter;
    uint16_t i;
    uint16_t j;

    pos += 2;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    vrl->axis_count = lxb_font_read_u16(&data);
    if (vrl->axis_count != mf->table_fvar->axis_count) {
        return LXB_STATUS_ERROR_UNEXPECTED_DATA;
    }

    vrl->region_count = lxb_font_read_u16(&data);

    pos += vrl->region_count * vrl->axis_count * 3 * 2;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    vrl->variation_regions = LXB_FONT_ALLOC(vrl->region_count,
                                            variation_region_t);
    if (vrl->variation_regions == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    iter = vrl->variation_regions;
    for (i = 0; i < vrl->region_count; i++, iter++) {
        iter->region_axes = LXB_FONT_ALLOC(vrl->axis_count, variation_region_t);
        if (iter->region_axes == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        for (j = 0; j < vrl->axis_count; j++) {
            iter->region_axes[j].start_coord = lxb_font_read_16(&data);
            iter->region_axes[j].peak_coord = lxb_font_read_16(&data);
            iter->region_axes[j].end_coord = lxb_font_read_16(&data);
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
item_variation_data_fill(lxb_font_t *mf,
                         uint32_t pos,
                         uint8_t *data,
                         size_t size,
                         item_variation_data_t *ivd)
{
    uint16_t i;

    pos += 6;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    ivd->item_count = lxb_font_read_u16(&data);
    ivd->short_delta_count = lxb_font_read_u16(&data);
    ivd->region_index_count = lxb_font_read_u16(&data);

    pos += ivd->region_index_count * 2;
    pos += ivd->short_delta_count * 2;
    pos += (ivd->item_count - ivd->short_delta_count);
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    ivd->region_indexes = LXB_FONT_ALLOC(ivd->region_index_count, uint16_t);
    if (ivd->region_indexes == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    ivd->delta_sets_16 = LXB_FONT_ALLOC(ivd->short_delta_count, int16_t);
    if (ivd->delta_sets_16 == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    ivd->delta_sets_8 = LXB_FONT_ALLOC(ivd->item_count - ivd->short_delta_count,
                                       int8_t);
    if (ivd->delta_sets_8 == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (i = 0; i < ivd->region_index_count; i++) {
        ivd->region_indexes[i] = lxb_font_read_u16(&data);
    }

    for (i = 0; i < ivd->short_delta_count; i++) {
        ivd->delta_sets_16[i] = lxb_font_read_16(&data);
    }

    for (i = 0; i < (ivd->item_count - ivd->short_delta_count); i++) {
        ivd->delta_sets_8[i] = lxb_font_read_8(&data);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_font_item_variation_store_fill(lxb_font_t *mf,
                                   uint32_t offset,
                                   uint8_t *data,
                                   size_t size,
                                   item_variation_store_t *ivs)
{
    item_variation_data_t *iter;
    uint8_t *ivd_data;
    uint8_t *vrl_data;
    lxb_status_t status;
    uint32_t pos;
    uint32_t vrl_offset;
    uint32_t ivd_offset;
    uint16_t format;
    uint16_t i;

    pos = offset + 8;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    format = lxb_font_read_u16(&data);
    if (format != 1) {
        return LXB_STATUS_ERROR_UNEXPECTED_DATA;
    }

    vrl_offset = lxb_font_read_u32(&data);
    ivs->item_variation_data_count = lxb_font_read_u16(&data);

    /* Item Variation Data. */
    pos += ivs->item_variation_data_count * 4;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    ivs->item_variation_data = LXB_FONT_ALLOC(ivs->item_variation_data_count,
                                              item_variation_data_t);
    if (ivs->item_variation_data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    iter = ivs->item_variation_data;
    for (i = 0; i < ivs->item_variation_data_count; i++, iter++) {
        ivd_offset = lxb_font_read_u32(&data);
        ivd_data = data + offset + ivd_offset;
        pos = offset + ivd_offset;

        status = item_variation_data_fill(mf, pos, ivd_data, size, iter);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    /* Variation Region List. */
    vrl_data = data + offset + vrl_offset;
    pos = offset + vrl_offset;

    status = variation_region_list_fill(mf, pos, vrl_data, size,
                                        &ivs->variation_region_list);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return LXB_STATUS_OK;
}
