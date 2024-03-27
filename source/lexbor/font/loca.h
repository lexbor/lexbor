/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_LOCA_H
#define LEXBOR_FONT_LOCA_H

#ifndef LEXBOR_FONT_HEAD_H
# error "head.h must be included before loca.h"
#endif

#ifndef LEXBOR_FONT_MAXP_H
# error "maxp.h must be included before loca.h"
#endif

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    uint32_t *offsets;
}
lxb_font_table_loca_t;

lxb_font_table_loca_t *
lxb_font_table_loca(lxb_font_t *mf, uint8_t* font_data, size_t size);

uint32_t
lxb_font_loca_get_offset(lxb_font_t *mf, uint16_t glyph_index);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_LOCA_H */
