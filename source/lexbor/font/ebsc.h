/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_EBSC_H
#define LEXBOR_FONT_EBSC_H

#include "lexbor/font/font_common.h"
#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    s_bit_line_metrics_t hori;
    s_bit_line_metrics_t vert;
    uint8_t ppem_x;
    uint8_t ppem_y;
    uint8_t substitute_ppem_x;
    uint8_t substitute_ppem_y;
}
ebsc_bitmap_scale_t;

typedef struct {
    /* 2 uint16_t minor and major versions ignored. */
    uint32_t num_sizes;
    ebsc_bitmap_scale_t *bitmap_scales;
}
lxb_font_table_ebsc_t;


lxb_font_table_ebsc_t *
lxb_font_table_ebsc(lxb_font_t *mf, uint8_t *font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_EBSC_H */
