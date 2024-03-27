/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_NAME_H
#define LEXBOR_FONT_NAME_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    uint16_t platform_id;
    uint16_t encoding_id;
    uint16_t language_id;
    uint16_t name_id;
    uint16_t length;
    uint16_t offset;
}
name_record_t;

typedef struct {
    uint16_t length;
    uint16_t offset;
}
name_lang_tag_record_t;

typedef struct {
    uint16_t format;
    uint16_t count;
    uint16_t string_offset;
    name_record_t *name_record;
    uint16_t lang_tag_count;
    name_lang_tag_record_t *lang_tag_record;
    char *str_data; /* FIXME: todo... */
}
lxb_font_table_name_t;

lxb_font_table_name_t *
lxb_font_table_name(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_NAME_H */
