/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_OS_2_H
#define LEXBOR_FONT_OS_2_H

#include "lexbor/font/font.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct lxb_font_s lxb_font_t;

typedef enum {
    LXB_FONT_OS_2_PANOSE_FAMILY_TYPE,
    LXB_FONT_OS_2_PANOSE_SERIF_STYLE,
    LXB_FONT_OS_2_PANOSE_WEIGHT,
    LXB_FONT_OS_2_PANOSE_PROPORTION,
    LXB_FONT_OS_2_PANOSE_CONTRAST,
    LXB_FONT_OS_2_PANOSE_STROKE_VARIATION,
    LXB_FONT_OS_2_PANOSE_ARM_STYLE,
    LXB_FONT_OS_2_PANOSE_LETTER_FORM,
    LXB_FONT_OS_2_PANOSE_MIDLINE,
    LXB_FONT_OS_2_PANOSE_X_HEIGHT
} lxb_font_os_2_panose_t;

typedef struct {
    /* version 0 */
    uint16_t version;
    int16_t x_avg_char_width;
    uint16_t us_weight_class;
    uint16_t us_width_class;
    uint16_t fs_type;
    int16_t y_subscript_x_size;
    int16_t y_subscript_y_size;
    int16_t y_subscript_x_offset;
    int16_t y_subscript_y_offset;
    int16_t y_superscript_x_size;
    int16_t y_superscript_y_size;
    int16_t y_superscript_x_offset;
    int16_t y_superscript_y_offset;
    int16_t y_strikeout_size;
    int16_t y_strikeout_position;
    int16_t s_family_class;
    uint8_t panose[10];
    uint32_t ul_unicode_range1;
    uint32_t ul_unicode_range2;
    uint32_t ul_unicode_range3;
    uint32_t ul_unicode_range4;
    uint8_t ach_vend_id[4];
    uint16_t fs_selection;
    uint16_t us_first_char_index;
    uint16_t us_last_char_index;
    int16_t s_typo_ascender;
    int16_t s_typo_descender;
    int16_t s_typo_line_gap;
    uint16_t us_win_ascent;
    uint16_t us_win_descent;
    /* version 1 */
    uint32_t ul_code_page_range1;
    uint32_t ul_code_page_range2;
    /* version 2, 3 and 4 */
    int16_t sx_height;
    int16_t s_cap_height;
    uint16_t us_default_char;
    uint16_t us_break_char;
    uint16_t us_max_context;
    /* version 5 */
    uint16_t us_lower_optical_point_size;
    uint16_t us_upper_optical_point_size;
}
lxb_font_table_os_2_t;

lxb_font_table_os_2_t *
lxb_font_table_os_2(lxb_font_t *mf, uint8_t* font_data, size_t size);

uint8_t lxb_font_os_2_get_panose(const void *mf, lxb_font_os_2_panose_t id);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_, FONT_OS_2_H */
