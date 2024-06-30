/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_MVAR_H
#define LEXBOR_FONT_MVAR_H

#include "lexbor/font/font.h"
#include "lexbor/font/font_common.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    uint8_t value_tag[4];
    uint16_t delta_set_outer_index;
    uint16_t delta_set_inner_index;
}
mvar_value_record_t;

typedef struct {
    /* 2 uint16_t minor and major versions ignored. */
    /* uint16 reserved ignored. */
    uint16_t value_record_size;
    uint16_t value_record_count;
    mvar_value_record_t *value_records;
    item_variation_store_t item_variation_store;
}
lxb_font_table_mvar_t;


lxb_font_table_mvar_t *
lxb_font_table_mvar(lxb_font_t *mf, uint8_t *font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_MVAR_H */
