/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_SVG_H
#define LEXBOR_FONT_SVG_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef struct {
    uint16_t start_glyph_id;
    uint16_t end_glyph_id;
    uint32_t svg_doc_offset;
    uint32_t svg_doc_length;
}
svg_document_record_t;

typedef struct {
    /* uint16_t version ignored. */
    uint32_t offset_to_svg_document_list;
    /* uint32_t reserved, ignored */
    uint16_t num_entries;
    svg_document_record_t *document_records;
}
lxb_font_table_svg_t;

lxb_font_table_svg_t *
lxb_font_table_svg(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_SVG_H */
