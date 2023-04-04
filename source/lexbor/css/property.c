/*
 * Copyright (C) 2021-2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/css/css.h"
#include "lexbor/css/property.h"
#include "lexbor/css/parser.h"
#include "lexbor/css/stylesheet.h"
#include "lexbor/css/property/state.h"
#include "lexbor/css/property/res.h"
#include "lexbor/core/serialize.h"
#include "lexbor/core/conv.h"


const lxb_css_entry_data_t *
lxb_css_property_by_name(const lxb_char_t *name, size_t length)
{
    const lexbor_shs_entry_t *entry;

    entry = lexbor_shs_entry_get_lower_static(lxb_css_property_shs,
                                              name, length);
    if (entry == NULL) {
        return NULL;
    }

    return entry->value;
}

const lxb_css_entry_data_t *
lxb_css_property_by_id(uintptr_t id)
{
    return &lxb_css_property_data[id];
}

void *
lxb_css_property_destroy(lxb_css_memory_t *memory, void *style,
                         lxb_css_property_type_t type, bool self_destroy)
{
    const lxb_css_entry_data_t *data;

    data = lxb_css_property_by_id(type);
    if (data == NULL) {
        return style;
    }

    return data->destroy(memory, style, self_destroy);
}

lxb_status_t
lxb_css_property_serialize(const void *style, lxb_css_property_type_t type,
                           lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_entry_data_t *data;

    data = lxb_css_property_by_id(type);
    if (data == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    return data->serialize(style, cb, ctx);
}

lxb_status_t
lxb_css_property_serialize_str(const void *style, lxb_css_property_type_t type,
                               lexbor_mraw_t *mraw, lexbor_str_t *str)
{
    const lxb_css_entry_data_t *data;

    data = lxb_css_property_by_id(type);
    if (data == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    return lxb_css_serialize_str_handler(style, str, mraw, data->serialize);
}

lxb_status_t
lxb_css_property_serialize_name(const void *style, lxb_css_property_type_t type,
                                lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_entry_data_t *data;

    switch (type) {
        case LXB_CSS_PROPERTY__UNDEF:
            return lxb_css_property__undef_serialize_name(style, cb, ctx);

        case LXB_CSS_PROPERTY__CUSTOM:
            return lxb_css_property__custom_serialize_name(style, cb, ctx);

        default:
            break;
    }

    data = lxb_css_property_by_id(type);
    if (data == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    return cb(data->name, data->length, ctx);
}

lxb_status_t
lxb_css_property_serialize_name_str(const void *style, lxb_css_property_type_t type,
                                    lexbor_mraw_t *mraw, lexbor_str_t *str)
{
    const lxb_css_entry_data_t *data;

    switch (type) {
        case LXB_CSS_PROPERTY__UNDEF:
            return lxb_css_serialize_str_handler(style, str, mraw,
                                       lxb_css_property__undef_serialize_name);

        case LXB_CSS_PROPERTY__CUSTOM:
            return lxb_css_serialize_str_handler(style, str, mraw,
                                      lxb_css_property__custom_serialize_name);

        default:
            break;
    }

    data = lxb_css_property_by_id(type);
    if (data == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    if (str->data == NULL) {
        lexbor_str_init(str, mraw, data->length);
        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    (void) lexbor_str_append(str, mraw, data->name, data->length);

    return LXB_STATUS_OK;
}

/* _undef. */

void *
lxb_css_property__undef_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property__undef_t));
}

void *
lxb_css_property__undef_destroy(lxb_css_memory_t *memory,
                                void *style, bool self_destroy)
{
    if (style == NULL) {
        return NULL;
    }

    if (self_destroy) {
        return lexbor_mraw_free(memory->mraw, style);
    }

    return style;
}

lxb_status_t
lxb_css_property__undef_serialize(const void *style,
                                  lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_property__undef_t *undef = style;

    return cb(undef->value.data, undef->value.length, ctx);
}

lxb_status_t
lxb_css_property__undef_serialize_name(const void *style,
                                       lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_property__undef_t *undef = style;
    const lxb_css_entry_data_t *data;

    if (undef->type == LXB_CSS_PROPERTY__UNDEF) {
        return LXB_STATUS_OK;
    }

    data = lxb_css_property_by_id(undef->type);
    if (data == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    return cb(data->name, data->length, ctx);
}

lxb_status_t
lxb_css_property__undef_serialize_value(const void *style,
                                        lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_property__undef_t *undef = style;

    if (undef->type == LXB_CSS_PROPERTY__UNDEF) {
        return cb(undef->value.data, undef->value.length, ctx);
    }

    return LXB_STATUS_OK;
}

/* _custom. */

void *
lxb_css_property__custom_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property__custom_t));
}

void *
lxb_css_property__custom_destroy(lxb_css_memory_t *memory,
                                 void *style, bool self_destroy)
{
    if (style == NULL) {
        return NULL;
    }

    if (self_destroy) {
        return lexbor_mraw_free(memory->mraw, style);
    }

    return style;
}

lxb_status_t
lxb_css_property__custom_serialize(const void *style,
                                   lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_property__custom_t *custom = style;

    if (custom->value.data == NULL) {
        return LXB_STATUS_OK;
    }

    return cb(custom->value.data, custom->value.length, ctx);
}

lxb_status_t
lxb_css_property__custom_serialize_name(const void *style,
                                        lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_property__custom_t *custom = style;

    return cb(custom->name.data, custom->name.length, ctx);
}

lxb_status_t
lxb_css_property__custom_serialize_value(const void *style,
                                         lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_property__custom_t *custom = style;

    if (custom->value.data == NULL) {
        return LXB_STATUS_OK;
    }

    return cb(custom->value.data, custom->value.length, ctx);
}

/* Display. */

void *
lxb_css_property_display_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_display_t));
}

void *
lxb_css_property_display_destroy(lxb_css_memory_t *memory,
                                 void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_display_serialize(const void *property,
                                   lexbor_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    const lxb_css_data_t *data;
    const lxb_css_property_display_t *display = property;

    static const lexbor_str_t str_ws = lexbor_str(" ");

    data = lxb_css_value_by_id(display->a);
    if (data == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    lexbor_serialize_write(cb, data->name, data->length, ctx, status);

    if (display->b == LXB_CSS_PROPERTY__UNDEF) {
        return LXB_STATUS_OK;
    }

    lexbor_serialize_write(cb, str_ws.data, str_ws.length, ctx, status);

    data = lxb_css_value_by_id(display->b);
    if (data == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    lexbor_serialize_write(cb, data->name, data->length, ctx, status);

    if (display->c == LXB_CSS_PROPERTY__UNDEF) {
        return LXB_STATUS_OK;
    }

    lexbor_serialize_write(cb, str_ws.data, str_ws.length, ctx, status);

    data = lxb_css_value_by_id(display->c);
    if (data == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    lexbor_serialize_write(cb, data->name, data->length, ctx, status);

    return LXB_STATUS_OK;
}


/* Width. */

void *
lxb_css_property_width_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_width_t));
}

void *
lxb_css_property_width_destroy(lxb_css_memory_t *memory,
                               void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_width_serialize(const void *property,
                                 lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_property_width_t *width = property;

    switch (width->type) {
        case LXB_CSS_VALUE_AUTO:
        case LXB_CSS_VALUE_MIN_CONTENT:
        case LXB_CSS_VALUE_MAX_CONTENT:
            return lxb_css_value_serialize(width->type, cb, ctx);

        case LXB_CSS_VALUE__LENGTH:
        case LXB_CSS_VALUE__NUMBER:
            return lxb_css_value_length_sr(&width->u.length, cb, ctx);

        case LXB_CSS_VALUE__PERCENTAGE:
            return lxb_css_value_percentage_sr(&width->u.percentage, cb, ctx);

        case LXB_CSS_VALUE__UNDEF:
            /* FIXME: ???? */
            break;

        default:
            return lxb_css_value_serialize(width->type, cb, ctx);
    }

    return LXB_STATUS_OK;
}

/* Height. */

void *
lxb_css_property_height_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_height_t));
}

void *
lxb_css_property_height_destroy(lxb_css_memory_t *memory,
                                void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_height_serialize(const void *property,
                                  lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_width_serialize(property, cb, ctx);
}

/* Box-sizing. */

void *
lxb_css_property_box_sizing_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_box_sizing_t));
}

void *
lxb_css_property_box_sizing_destroy(lxb_css_memory_t *memory,
                                    void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_box_sizing_serialize(const void *property,
                                      lexbor_serialize_cb_f cb, void *ctx)
{
    const lxb_css_property_box_sizing_t *bsize = property;

    return lxb_css_value_serialize(bsize->type, cb, ctx);
}

/* Min-width. */

void *
lxb_css_property_min_width_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_min_width_t));
}

void *
lxb_css_property_min_width_destroy(lxb_css_memory_t *memory,
                                   void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_min_width_serialize(const void *property,
                                     lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_width_serialize(property, cb, ctx);
}

/* Min-height. */

void *
lxb_css_property_min_height_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_min_height_t));
}

void *
lxb_css_property_min_height_destroy(lxb_css_memory_t *memory,
                                    void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_min_height_serialize(const void *property,
                                      lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_width_serialize(property, cb, ctx);
}

/* Max-width. */

void *
lxb_css_property_max_width_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_max_width_t));
}

void *
lxb_css_property_max_width_destroy(lxb_css_memory_t *memory,
                                   void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_max_width_serialize(const void *property,
                                     lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_width_serialize(property, cb, ctx);
}

/* Max-height. */

void *
lxb_css_property_max_height_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_max_height_t));
}

void *
lxb_css_property_max_height_destroy(lxb_css_memory_t *memory,
                                    void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_max_height_serialize(const void *property,
                                      lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_width_serialize(property, cb, ctx);
}

/* Margin. */

void *
lxb_css_property_margin_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_margin_t));
}

void *
lxb_css_property_margin_destroy(lxb_css_memory_t *memory,
                                void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_margin_serialize(const void *property,
                                  lexbor_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    const lxb_css_property_margin_t *margin = property;

    static const lexbor_str_t str_ws = lexbor_str(" ");

    /* Top. */

    status = lxb_css_value_length_percentage_sr(&margin->top, cb, ctx);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (margin->right.type == LXB_CSS_VALUE__UNDEF) {
        return LXB_STATUS_OK;
    }

    /* Right. */

    lexbor_serialize_write(cb, str_ws.data, str_ws.length, ctx, status);

    status = lxb_css_value_length_percentage_sr(&margin->right, cb, ctx);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (margin->bottom.type == LXB_CSS_VALUE__UNDEF) {
        return LXB_STATUS_OK;
    }

    /* Bottom. */

    lexbor_serialize_write(cb, str_ws.data, str_ws.length, ctx, status);

    status = lxb_css_value_length_percentage_sr(&margin->bottom, cb, ctx);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (margin->left.type == LXB_CSS_VALUE__UNDEF) {
        return LXB_STATUS_OK;
    }

    /* Left. */

    lexbor_serialize_write(cb, str_ws.data, str_ws.length, ctx, status);

    return lxb_css_value_length_percentage_sr(&margin->left, cb, ctx);
}

/* Margin-top. */

void *
lxb_css_property_margin_top_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_margin_top_t));
}

void *
lxb_css_property_margin_top_destroy(lxb_css_memory_t *memory,
                                    void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_margin_top_serialize(const void *property,
                                      lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_value_length_percentage_sr(property, cb, ctx);
}

/* Margin-right. */

void *
lxb_css_property_margin_right_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_margin_right_t));
}

void *
lxb_css_property_margin_right_destroy(lxb_css_memory_t *memory,
                                      void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_margin_right_serialize(const void *property,
                                        lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_value_length_percentage_sr(property, cb, ctx);
}

/* Margin-bottom. */

void *
lxb_css_property_margin_bottom_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_margin_bottom_t));
}

void *
lxb_css_property_margin_bottom_destroy(lxb_css_memory_t *memory,
                                       void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_margin_bottom_serialize(const void *property,
                                         lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_value_length_percentage_sr(property, cb, ctx);
}

/* Margin-left. */

void *
lxb_css_property_margin_left_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_margin_left_t));
}

void *
lxb_css_property_margin_left_destroy(lxb_css_memory_t *memory,
                                     void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_margin_left_serialize(const void *property,
                                       lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_value_length_percentage_sr(property, cb, ctx);
}

/* Padding. */

void *
lxb_css_property_padding_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_padding_t));
}

void *
lxb_css_property_padding_destroy(lxb_css_memory_t *memory,
                                 void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_padding_serialize(const void *property,
                                   lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_margin_serialize(property, cb, ctx);
}

/* Padding-top. */

void *
lxb_css_property_padding_top_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_padding_top_t));
}

void *
lxb_css_property_padding_top_destroy(lxb_css_memory_t *memory,
                                     void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_padding_top_serialize(const void *property,
                                       lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_value_length_percentage_sr(property, cb, ctx);
}

/* Padding-right. */

void *
lxb_css_property_padding_right_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_padding_right_t));
}

void *
lxb_css_property_padding_right_destroy(lxb_css_memory_t *memory,
                                       void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_padding_right_serialize(const void *property,
                                         lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_value_length_percentage_sr(property, cb, ctx);
}

/* Padding-bottom. */

void *
lxb_css_property_padding_bottom_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_padding_bottom_t));
}

void *
lxb_css_property_padding_bottom_destroy(lxb_css_memory_t *memory,
                                        void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_padding_bottom_serialize(const void *property,
                                          lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_value_length_percentage_sr(property, cb, ctx);
}

/* Padding-left. */

void *
lxb_css_property_padding_left_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_padding_left_t));
}

void *
lxb_css_property_padding_left_destroy(lxb_css_memory_t *memory,
                                      void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_padding_left_serialize(const void *property,
                                        lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_value_length_percentage_sr(property, cb, ctx);
}

/* Border. */

void *
lxb_css_property_border_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_border_t));
}

void *
lxb_css_property_border_destroy(lxb_css_memory_t *memory,
                                void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_border_serialize(const void *property,
                                  lexbor_serialize_cb_f cb, void *ctx)
{
    bool ws_print;
    lxb_status_t status;
    const lxb_css_property_border_t *border = property;

    static const lexbor_str_t str_ws = lexbor_str(" ");

    ws_print = false;

    if (border->width.type != LXB_CSS_VALUE__UNDEF) {
        status = lxb_css_value_length_type_sr(&border->width, cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        ws_print = true;
    }

    if (border->style != LXB_CSS_VALUE__UNDEF) {
        if (ws_print) {
            lexbor_serialize_write(cb, str_ws.data, str_ws.length, ctx, status);
        }

        status = lxb_css_value_serialize(border->style, cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        ws_print = true;
    }

    if (border->color.type != LXB_CSS_VALUE__UNDEF) {
        if (ws_print) {
            lexbor_serialize_write(cb, str_ws.data, str_ws.length, ctx, status);
        }

        return lxb_css_value_color_serialize(&border->color, cb, ctx);
    }

    return LXB_STATUS_OK;
}

/* Border-top. */

void *
lxb_css_property_border_top_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_border_top_t));
}

void *
lxb_css_property_border_top_destroy(lxb_css_memory_t *memory,
                                    void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_border_top_serialize(const void *property,
                                      lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_border_serialize(property, cb, ctx);
}

/* Border-right. */

void *
lxb_css_property_border_right_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_border_right_t));
}

void *
lxb_css_property_border_right_destroy(lxb_css_memory_t *memory,
                                      void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_border_right_serialize(const void *property,
                                        lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_border_serialize(property, cb, ctx);
}

/* Border-bottom. */

void *
lxb_css_property_border_bottom_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_border_bottom_t));
}

void *
lxb_css_property_border_bottom_destroy(lxb_css_memory_t *memory,
                                       void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_border_bottom_serialize(const void *property,
                                         lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_border_serialize(property, cb, ctx);
}

/* Border-left. */

void *
lxb_css_property_border_left_create(lxb_css_memory_t *memory)
{
    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_border_left_t));
}

void *
lxb_css_property_border_left_destroy(lxb_css_memory_t *memory,
                                     void *property, bool self_destroy)
{
    return lxb_css_property__undef_destroy(memory, property, self_destroy);
}

lxb_status_t
lxb_css_property_border_left_serialize(const void *property,
                                       lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_css_property_border_serialize(property, cb, ctx);
}
