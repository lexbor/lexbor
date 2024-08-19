/*
 * Copyright (C) 2020 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_COMMON_H
#define LEXBOR_FONT_COMMON_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Used in eblc, ebsc and cblc tables. */
typedef struct {
    int8_t ascender;
    int8_t descender;
    uint8_t width_max;
    int8_t caret_slope_numerator;
    int8_t caret_slope_denominator;
    int8_t caret_offset;
    int8_t min_origin_sb;
    int8_t min_advance_sb;
    int8_t max_before_bl;
    int8_t min_after_bl;
    int8_t pad1;
    int8_t pad2;
}
s_bit_line_metrics_t;

typedef struct {
    int16_t start_coord;
    int16_t peak_coord;
    int16_t end_coord;
}
region_axis_coordinates_t;

typedef struct {
    region_axis_coordinates_t *region_axes;
}
variation_region_t;

typedef struct {
    uint16_t axis_count;
    uint16_t region_count;
    variation_region_t *variation_regions;
}
variation_region_list_t;

typedef struct {
    uint16_t item_count;
    uint16_t short_delta_count;
    uint16_t region_index_count;
    uint16_t *region_indexes;
    int16_t *delta_sets_16; /* short_delta_count */
    int8_t *delta_sets_8; /* item_count - short_delta_count */
}
item_variation_data_t;

/* Used in base, gdef, hvar, mvar and vvar tables. */
typedef struct {
    /* uint16_t format ignored. */
    uint16_t item_variation_data_count;
    variation_region_list_t variation_region_list;
    item_variation_data_t *item_variation_data;
}
item_variation_store_t;

void
lxb_font_s_bit_line_metrics_fill(uint8_t *data,
                                 s_bit_line_metrics_t *metrics);

lxb_status_t
lxb_font_item_variation_store_fill(lxb_font_t *mf,
                                   uint32_t offset,
                                   uint8_t *data,
                                   size_t size,
                                   item_variation_store_t *ivs);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_COMMON_H */
