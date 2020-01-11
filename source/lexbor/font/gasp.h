/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_GASP_H
#define LEXBOR_FONT_GASP_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    uint16_t range_max_ppem;
    uint16_t range_gasp_behavior;
}
gasp_range_t;

typedef struct {
    /* uint16_t version ignored. */
    uint16_t num_ranges;
    gasp_range_t *gasp_ranges;
}
lxb_font_table_gasp_t;

lxb_font_table_gasp_t *
lxb_font_table_gasp(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_GASP_H */
