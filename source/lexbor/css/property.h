/*
 * Copyright (C) 2021-2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LXB_CSS_PROPERTY_H
#define LXB_CSS_PROPERTY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/serialize.h"
#include "lexbor/css/base.h"
#include "lexbor/css/value.h"
#include "lexbor/css/unit/const.h"
#include "lexbor/css/property/const.h"


typedef struct {
    lxb_css_property_type_t type;
    lexbor_str_t            value;
}
lxb_css_property__undef_t;

typedef struct {
    lexbor_str_t name;
    lexbor_str_t value;
}
lxb_css_property__custom_t;

typedef struct {
    lxb_css_display_type_t a;
    lxb_css_display_type_t b;
    lxb_css_display_type_t c;
}
lxb_css_property_display_t;

typedef lxb_css_value_length_percentage_t lxb_css_property_width_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_height_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_min_width_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_min_height_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_max_width_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_max_height_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_margin_top_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_margin_right_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_margin_bottom_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_margin_left_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_padding_top_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_padding_right_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_padding_bottom_t;
typedef lxb_css_value_length_percentage_t lxb_css_property_padding_left_t;

typedef struct {
    lxb_css_box_sizing_type_t type;
}
lxb_css_property_box_sizing_t;

typedef struct {
    lxb_css_property_margin_top_t    top;
    lxb_css_property_margin_right_t  right;
    lxb_css_property_margin_bottom_t bottom;
    lxb_css_property_margin_left_t   left;
}
lxb_css_property_margin_t;

typedef struct {
    lxb_css_property_padding_top_t    top;
    lxb_css_property_padding_right_t  right;
    lxb_css_property_padding_bottom_t bottom;
    lxb_css_property_padding_left_t   left;
}
lxb_css_property_padding_t;

typedef struct {
    lxb_css_value_type_t        style;
    lxb_css_value_length_type_t width;
    lxb_css_value_color_t       color;
}
lxb_css_property_border_t;

typedef lxb_css_property_border_t lxb_css_property_border_top_t;
typedef lxb_css_property_border_t lxb_css_property_border_right_t;
typedef lxb_css_property_border_t lxb_css_property_border_bottom_t;
typedef lxb_css_property_border_t lxb_css_property_border_left_t;


LXB_API const lxb_css_entry_data_t *
lxb_css_property_by_name(const lxb_char_t *name, size_t length);

LXB_API const lxb_css_entry_data_t *
lxb_css_property_by_id(uintptr_t id);

LXB_API void *
lxb_css_property_destroy(lxb_css_memory_t *memory, void *style,
                         lxb_css_property_type_t type, bool self_destroy);

LXB_API lxb_status_t
lxb_css_property_serialize(const void *style, lxb_css_property_type_t type,
                           lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_status_t
lxb_css_property_serialize_str(const void *style, lxb_css_property_type_t type,
                               lexbor_mraw_t *mraw, lexbor_str_t *str);

LXB_API lxb_status_t
lxb_css_property_serialize_name(const void *style, lxb_css_property_type_t type,
                                lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_status_t
lxb_css_property_serialize_name_str(const void *style, lxb_css_property_type_t type,
                                    lexbor_mraw_t *mraw, lexbor_str_t *str);

/* _undef. */

LXB_API void *
lxb_css_property__undef_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property__undef_destroy(lxb_css_memory_t *memory,
                                void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property__undef_serialize(const void *style,
                                  lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_status_t
lxb_css_property__undef_serialize_name(const void *style,
                                       lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_status_t
lxb_css_property__undef_serialize_value(const void *style,
                                        lexbor_serialize_cb_f cb, void *ctx);

/* _custom. */

LXB_API void *
lxb_css_property__custom_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property__custom_destroy(lxb_css_memory_t *memory,
                                 void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property__custom_serialize(const void *style,
                                   lexbor_serialize_cb_f cb, void *ctx);
LXB_API lxb_status_t
lxb_css_property__custom_serialize_name(const void *style,
                                        lexbor_serialize_cb_f cb, void *ctx);
LXB_API lxb_status_t
lxb_css_property__custom_serialize_value(const void *style,
                                         lexbor_serialize_cb_f cb, void *ctx);

/* Display. */

LXB_API void *
lxb_css_property_display_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_display_destroy(lxb_css_memory_t *memory,
                                 void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_display_serialize(const void *style,
                                   lexbor_serialize_cb_f cb, void *ctx);

/* Width. */

LXB_API void *
lxb_css_property_width_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_width_destroy(lxb_css_memory_t *memory,
                               void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_width_serialize(const void *style,
                                 lexbor_serialize_cb_f cb, void *ctx);

/* Height. */

LXB_API void *
lxb_css_property_height_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_height_destroy(lxb_css_memory_t *memory,
                                void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_height_serialize(const void *style,
                                  lexbor_serialize_cb_f cb, void *ctx);

/* Box-sizing. */

LXB_API void *
lxb_css_property_box_sizing_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_box_sizing_destroy(lxb_css_memory_t *memory,
                                    void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_box_sizing_serialize(const void *style,
                                      lexbor_serialize_cb_f cb, void *ctx);

/* Min-width. */

LXB_API void *
lxb_css_property_min_width_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_min_width_destroy(lxb_css_memory_t *memory,
                                   void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_min_width_serialize(const void *style,
                                     lexbor_serialize_cb_f cb, void *ctx);

/* Min-height. */

LXB_API void *
lxb_css_property_min_height_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_min_height_destroy(lxb_css_memory_t *memory,
                                    void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_min_height_serialize(const void *style,
                                      lexbor_serialize_cb_f cb, void *ctx);

/* Max-width. */

LXB_API void *
lxb_css_property_max_width_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_max_width_destroy(lxb_css_memory_t *memory,
                                   void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_max_width_serialize(const void *style,
                                     lexbor_serialize_cb_f cb, void *ctx);

/* Max-height. */

LXB_API void *
lxb_css_property_max_height_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_max_height_destroy(lxb_css_memory_t *memory,
                                    void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_max_height_serialize(const void *style,
                                      lexbor_serialize_cb_f cb, void *ctx);

/* Margin. */

LXB_API void *
lxb_css_property_margin_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_margin_destroy(lxb_css_memory_t *memory,
                                void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_margin_serialize(const void *style,
                                  lexbor_serialize_cb_f cb, void *ctx);

/* Margin-top. */

LXB_API void *
lxb_css_property_margin_top_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_margin_top_destroy(lxb_css_memory_t *memory,
                                    void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_margin_top_serialize(const void *style,
                                      lexbor_serialize_cb_f cb, void *ctx);

/* Margin-right. */

LXB_API void *
lxb_css_property_margin_right_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_margin_right_destroy(lxb_css_memory_t *memory,
                                      void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_margin_right_serialize(const void *style,
                                        lexbor_serialize_cb_f cb, void *ctx);

/* Margin-bottom. */

LXB_API void *
lxb_css_property_margin_bottom_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_margin_bottom_destroy(lxb_css_memory_t *memory,
                                       void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_margin_bottom_serialize(const void *style,
                                         lexbor_serialize_cb_f cb, void *ctx);

/* Margin-left. */

LXB_API void *
lxb_css_property_margin_left_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_margin_left_destroy(lxb_css_memory_t *memory,
                                     void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_margin_left_serialize(const void *style,
                                       lexbor_serialize_cb_f cb, void *ctx);

/* Padding. */

LXB_API void *
lxb_css_property_padding_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_padding_destroy(lxb_css_memory_t *memory,
                                 void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_padding_serialize(const void *style,
                                   lexbor_serialize_cb_f cb, void *ctx);

/* Padding-top. */

LXB_API void *
lxb_css_property_padding_top_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_padding_top_destroy(lxb_css_memory_t *memory,
                                     void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_padding_top_serialize(const void *style,
                                       lexbor_serialize_cb_f cb, void *ctx);

/* Padding-right. */

LXB_API void *
lxb_css_property_padding_right_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_padding_right_destroy(lxb_css_memory_t *memory,
                                       void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_padding_right_serialize(const void *style,
                                         lexbor_serialize_cb_f cb, void *ctx);

/* Padding-bottom. */

LXB_API void *
lxb_css_property_padding_bottom_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_padding_bottom_destroy(lxb_css_memory_t *memory,
                                        void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_padding_bottom_serialize(const void *style,
                                          lexbor_serialize_cb_f cb, void *ctx);

/* Padding-left. */

LXB_API void *
lxb_css_property_padding_left_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_padding_left_destroy(lxb_css_memory_t *memory,
                                      void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_padding_left_serialize(const void *style,
                                        lexbor_serialize_cb_f cb, void *ctx);

/* Border. */

LXB_API void *
lxb_css_property_border_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_border_destroy(lxb_css_memory_t *memory,
                                void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_border_serialize(const void *style,
                                  lexbor_serialize_cb_f cb, void *ctx);

/* Border-top. */

LXB_API void *
lxb_css_property_border_top_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_border_top_destroy(lxb_css_memory_t *memory,
                                    void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_border_top_serialize(const void *style,
                                      lexbor_serialize_cb_f cb, void *ctx);

/* Border-right. */

LXB_API void *
lxb_css_property_border_right_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_border_right_destroy(lxb_css_memory_t *memory,
                                      void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_border_right_serialize(const void *style,
                                        lexbor_serialize_cb_f cb, void *ctx);

/* Border-bottom. */

LXB_API void *
lxb_css_property_border_bottom_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_border_bottom_destroy(lxb_css_memory_t *memory,
                                       void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_border_bottom_serialize(const void *style,
                                         lexbor_serialize_cb_f cb, void *ctx);

/* Border-left. */

LXB_API void *
lxb_css_property_border_left_create(lxb_css_memory_t *memory);

LXB_API void *
lxb_css_property_border_left_destroy(lxb_css_memory_t *memory,
                                     void *style, bool self_destroy);
LXB_API lxb_status_t
lxb_css_property_border_left_serialize(const void *style,
                                       lexbor_serialize_cb_f cb, void *ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LXB_CSS_PROPERTY_H */
