/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_GLYF_H
#define LEXBOR_FONT_GLYF_H

#ifndef LEXBOR_FONT_MAXP_H
# error "maxp.h must be included before glyf.h"
#endif

#ifndef LEXBOR_FONT_LOCA_H
# error "loca.h must be included before glyf.h"
#endif

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    LXB_FONT_ON_CURVE_POINT = 1 << 0,
    LXB_FONT_X_SHORT_VECTOR = 1 << 1,
    LXB_FONT_Y_SHORT_VECTOR = 1 << 2,
    LXB_FONT_REPEAT_FLAG = 1 << 3,
    LXB_FONT_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR = 1 << 4,
    LXB_FONT_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR = 1 << 5,
    LXB_FONT_OVERLAP_SIMPLE = 1 << 6
}
lxb_font_simple_glyph_flags_t;

typedef enum {
    LXB_FONT_ARG_1_AND_2_ARE_WORDS = 1 << 0,
    LXB_FONT_ARGS_ARE_XY_VALUES = 1 << 1,
    LXB_FONT_ROUND_XY_TO_GRID = 1 << 2,
    LXB_FONT_WE_HAVE_A_SCALE = 1 << 3,
    /* 0x10 is reserved. */
    LXB_FONT_MORE_COMPONENTS = 1 << 5,
    LXB_FONT_WE_HAVE_AN_X_AND_Y_SCALE = 1 << 6,
    LXB_FONT_WE_HAVE_A_TWO_BY_TWO = 1 << 7,
    LXB_FONT_WE_HAVE_INSTRUCTIONS = 1 << 8,
    LXB_FONT_USE_MY_METRICS = 1 << 9,
    LXB_FONT_OVERLAP_COMPOUND = 1 << 10,
    LXB_FONT_SCALED_COMPONENT_OFFSET = 1 << 11,
    LXB_FONT_UNSCALED_COMPONENT_OFFSET = 1 << 12
}
lxb_font_composite_glyph_flags_t;

typedef struct lxb_font_s lxb_font_t;

typedef struct {
    uint16_t *end_pts_of_contours;
    uint16_t instruction_length;
    uint8_t *instructions;
    uint16_t flags_count;
    uint8_t *flags;
    int16_t *x_coordinates;
    int16_t *y_coordinates;
}
glyf_simple_glyph_t;

typedef struct {
    uint16_t flags;
    uint16_t glyph_index;
    uint16_t argument1;
    uint16_t argument2;
}
glyf_composite_glyph_t;

typedef struct {
    /* Header. */
    int16_t number_of_contours;
    int16_t x_min;
    int16_t y_min;
    int16_t x_max;
    int16_t y_max;
    glyf_simple_glyph_t simple_glyph;
    glyf_composite_glyph_t composite_glyph;
}
glyph_t;

typedef struct {
    glyph_t *glyphs;
}
lxb_font_table_glyf_t;

lxb_font_table_glyf_t*
lxb_font_table_glyf(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_GLYF_H */
