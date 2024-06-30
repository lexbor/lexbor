/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_HMTX_H
#define LEXBOR_FONT_HMTX_H

#ifndef LEXBOR_FONT_HHEA_H
# error "hhea.h must be included before hmtx.h"
#endif

#ifndef LEXBOR_FONT_MAXP_H
# error "maxp.h must be included before hmtx.h"
#endif

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    uint16_t advance_width;
    int16_t lsb;
}
hmtx_long_hor_metric_t;

typedef struct {
    hmtx_long_hor_metric_t *h_metrics;
    int16_t *left_side_bearings;
}
lxb_font_table_hmtx_t;

lxb_font_table_hmtx_t*
lxb_font_table_hmtx(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_HMTX_H */
