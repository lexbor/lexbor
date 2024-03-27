/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_HHEA_H
#define LEXBOR_FONT_HHEA_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    /* 2 uint16_t minor and major versions ignored */
    int16_t ascender;
    int16_t descender;
    int16_t line_gap;
    uint16_t advanced_width_max;
    int16_t min_left_side_bearing;
    int16_t min_right_side_bearing;
    int16_t x_max_extent;
    int16_t caret_slope_rise;
    int16_t caret_slope_run;
    int16_t caret_offset;
    /* the 4 reserved int16_t ignored */
    int16_t metric_data_format;
    uint16_t number_of_h_metrics;
}
lxb_font_table_hhea_t;

lxb_font_table_hhea_t *
lxb_font_table_hhea(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_HHEA_H */
