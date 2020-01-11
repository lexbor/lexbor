/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_EBSC_H
#define LEXBOR_FONT_EBSC_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

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
s_bit_line_metrics;

typedef struct {
    s_bit_line_metrics hori;
    s_bit_line_metrics vert;
    uint8_t ppem_x;
    uint8_t ppem_y;
    uint8_t substitute_ppem_x;
    uint8_t substitute_ppem_y;
}
bitmap_scale_t;

typedef struct {
    /* 2 uint16_t minor and major versions ignored. */
    uint32_t num_sizes;
    bitmap_scale_t *bitmap_scales;
}
lxb_font_table_ebsc_t;


lxb_font_table_ebsc_t *
lxb_font_table_ebsc(lxb_font_t *mf, uint8_t *font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_EBSC_H */
