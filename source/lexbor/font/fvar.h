/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_FVAR_H
#define LEXBOR_FONT_FVAR_H

#include "lexbor/font/font.h"
#include "lexbor/font/font_common.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    uint16_t subfamily_name_id;
    uint16_t flags;
    int32_t *coordinates;
    uint16_t postscript_name_id;
}
fvar_instance_t;

typedef struct {
    int8_t axis_tag[4];
    int32_t min_value;
    int32_t default_value;
    int32_t max_value;
    uint16_t flags;
    uint16_t axis_name_id;
}
fvar_variation_axis_t;

typedef struct {
    /* 2 uint16_t minor and major versions ignored. */
    /* uint16 reserved ignored. */
    uint16_t axis_count;
    uint16_t axis_size;
    uint16_t instance_count;
    uint16_t instance_size;
    fvar_variation_axis_t *variation_axis;
    fvar_instance_t *instances;
}
lxb_font_table_fvar_t;


lxb_font_table_fvar_t *
lxb_font_table_fvar(lxb_font_t *mf, uint8_t *font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_FVAR_H */
