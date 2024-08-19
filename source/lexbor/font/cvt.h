/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_CVT_H
#define LEXBOR_FONT_CVT_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    int16_t *values;
}
lxb_font_table_cvt_t;

lxb_font_table_cvt_t *
lxb_font_table_cvt(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_CVT_H */
