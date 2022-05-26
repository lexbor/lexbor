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
    lxb_css_width_type_t type;

    union {
        lxb_css_value_length_t     number;
        lxb_css_value_percentage_t percentage;
    } u;
}
lxb_css_property_width_t;

typedef lxb_css_property_width_t lxb_css_property_height_t;


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


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LXB_CSS_PROPERTY_H */
