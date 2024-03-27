/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_POST_H
#define LEXBOR_FONT_POST_H

#ifndef LEXBOR_FONT_MAXP_H
# error "maxp.h must be included before post.h"
#endif

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    int32_t version;
    int32_t italic_angle;
    int16_t underline_position;
    int16_t underline_thickness;
    uint32_t is_fixed_pitch;
    uint32_t min_mem_type42;
    uint32_t max_mem_type42;
    uint32_t min_mem_type1;
    uint32_t max_mem_type1;
    /* version 2.0 or 2.5 */
    uint16_t num_glyphs;
    /* only version 2.0 */
    uint16_t *glyph_name_index;
    int8_t *names;
    /* only version 2.5 (deprecated as of OpenType 1.3) */
    int8_t *offset;
}
lxb_font_table_post_t;

lxb_font_table_post_t *
lxb_font_table_post(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_POST_H */
