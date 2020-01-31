/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_CMAP_H
#define LEXBOR_FONT_CMAP_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

/***** Format 0 *****/

typedef struct {
    /* uint16_t format ignored (value must be 0) */
    uint16_t length;
    uint16_t language;
    uint8_t glyph_id_array[256];
}
cmap_subtable_format_0_t;

/***** Format 2 *****/

typedef struct {
    uint16_t first_code;
    uint16_t entry_count;
    int16_t id_delta;
    uint16_t id_range_offset;
}
cmap_subtable_subheader_format_2_t;

typedef struct {
    /* uint16_t format ignored (value must be 2) */
    uint16_t length;
    uint16_t language;
    uint16_t sub_header_keys[256];
    cmap_subtable_subheader_format_2_t *subheaders;
    uint16_t *glyph_index_array;
}
cmap_subtable_format_2_t;

/***** Format 4 *****/

typedef struct {
    /* uint16_t format ignored (value must be 4) */
    uint16_t length;
    uint16_t language;
    uint16_t seg_count_x_2;
    uint16_t search_range;
    uint16_t entry_selector;
    uint16_t range_shift;
    uint16_t *end_code;
    /* uint16_t reserved value (must be 0) */
    uint16_t *start_code;
    int16_t *id_delta;
    uint16_t *id_range_offset;
    uint16_t glyph_id_count;
    uint16_t *glyph_id_array;
}
cmap_subtable_format_4_t;

/***** Format 6 *****/

typedef struct {
    /* uint16_t format ignored (value must be 6) */
    uint16_t length;
    uint16_t language;
    uint16_t first_code;
    uint16_t entry_count;
    uint16_t *glyph_id_array;
}
cmap_subtable_format_6_t;

/***** Format 8 *****/

typedef struct {
    uint32_t start_char_code;
    uint32_t end_char_code;
    uint32_t start_glyph_id;
}
cmap_sequential_map_group_t;

typedef struct {
    /* uint16_t format ignored (value must be 8) */
    /* uint16_t reserved ignored (value must be 0) */
    uint32_t length;
    uint32_t language;
    uint8_t is_32[8192];
    uint32_t num_groups;
    cmap_sequential_map_group_t *groups;
}
cmap_subtable_format_8_t;

/***** Format 10 *****/

typedef struct {
    /* uint16_t format ignored (value must be 10) */
    /* uint16_t reserved ignored (value must be 0) */
    uint32_t length;
    uint32_t language;
    uint32_t start_char_code;
    uint32_t num_chars;
    uint16_t *glyphs;
}
cmap_subtable_format_10_t;

/***** Format 12 *****/

typedef struct {
    /* uint16_t format ignored (value must be 12) */
    /* uint16_t reserved ignored (value must be 0) */
    uint32_t length;
    uint32_t language;
    uint32_t num_groups;
    cmap_sequential_map_group_t *groups;
}
cmap_subtable_format_12_t;

/***** Format 13 *****/

typedef struct {
    uint32_t start_char_code;
    uint32_t end_char_code;
    uint32_t glyph_id;
}
cmap_constant_map_group_t;

typedef struct {
    /* uint16_t format ignored (value must be 13) */
    /* uint16_t reserved ignored (value must be 0) */
    uint32_t length;
    uint32_t language;
    uint32_t num_groups;
    cmap_constant_map_group_t *groups;
}
cmap_subtable_format_13_t;

/***** Format 14 *****/

typedef struct {
    uint32_t start_unicode_value; /* is a uint24 in spec */
    uint8_t additional_count;
}
cmap_unicode_range_t;

typedef struct {
    uint32_t num_unicode_value_ranges;
    cmap_unicode_range_t *ranges;
}
cmap_default_uvs_t;

typedef struct {
    uint32_t unicode_value; /* is a uint24 in spec */
    uint8_t glyph_id;
}
cmap_uvs_mapping_t;

typedef struct {
    uint32_t num_uvs_mappings;
    cmap_uvs_mapping_t *uvs_mappings;
}
cmap_non_default_uvs_t;

typedef struct {
    uint32_t var_selector; /* is a uint24 in spec */
    uint32_t default_uvs_offset;
    uint32_t non_default_uvs_offset;
    cmap_default_uvs_t default_uvs;
    cmap_non_default_uvs_t non_default_uvs;
}
cmap_variation_selector_t;

typedef struct {
    /* uint16_t format ignored (value must be 14) */
    uint32_t length;
    uint32_t num_var_selector_records;
    cmap_variation_selector_t *var_selector;
}
cmap_subtable_format_14_t;

/***** Header *****/

typedef struct {
    uint16_t platform_id;
    uint16_t encoding_id;
    uint32_t offset;
}
cmap_encoding_record_t;

typedef struct {
    uint16_t format;
    void *header;
} cmap_subtable_t;

typedef struct {
    /* 1 uint16_t version ignored. */
    uint16_t num_tables;
    cmap_encoding_record_t *encoding_records;
    cmap_subtable_t *subtables;
}
lxb_font_table_cmap_t;


lxb_font_table_cmap_t *
lxb_font_table_cmap(lxb_font_t *mf, uint8_t* font_data, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_CMAP_H */
