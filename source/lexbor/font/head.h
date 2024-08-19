/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_HEAD_H
#define LEXBOR_FONT_HEAD_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    /* 2 uint16_t minor and major versions ignored. */
    int32_t font_revision;
    uint32_t check_sum_adjustment;

    /* uint32 magic number ignored. */
    uint16_t flags;
    uint16_t units_per_em;

    /* 2 int64_t created and modified time font ignored. */
    int16_t xmin;
    int16_t ymin;
    int16_t xmax;
    int16_t ymax;
    uint16_t mac_style;
    uint16_t lowest_rec_ppem;
    int16_t font_direction_hint; /* deprecated, see spec */
    int16_t index_to_loc_format;
    int16_t glyph_data_format;
}
lxb_font_table_head_t;


lxb_font_table_head_t *
lxb_font_table_head(lxb_font_t *mf, uint8_t *font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_HEAD_H */
