/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_SBIX_H
#define LEXBOR_FONT_SBIX_H

#ifndef LEXBOR_FONT_MAXP_H
# error "maxp.h must be included before sbix .h"
#endif

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    int16_t origin_offset_x;
    int16_t origin_offset_y;
    char graphic_type[4];
    uint8_t *data;
}
sbix_glyph_data_t;

typedef struct {
    uint16_t ppem;
    uint16_t ppi;
    sbix_glyph_data_t *glyphs;
}
sbix_strike_t;

typedef struct {
    /* uint16_t version ignored. */
    uint16_t flags;
    uint32_t num_strikes;
    sbix_strike_t *strikes;
}
lxb_font_table_sbix_t;

lxb_font_table_sbix_t *
lxb_font_table_sbix(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_SBIX_H */
