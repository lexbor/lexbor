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
    size_t length;
    lxb_char_t buf[128];
    lxb_status_t status;
    const lxb_css_data_t *unit;
    const lxb_css_value_length_t *num;
    const lxb_css_property_width_t *width = property;

    static const lexbor_str_t str_auto = lexbor_str("auto");
    static const lexbor_str_t str_min = lexbor_str("min-content");
    static const lexbor_str_t str_max = lexbor_str("max-content");
    static const lexbor_str_t str_per = lexbor_str("%");

    switch (width->type) {
        case LXB_CSS_VALUE_AUTO:
            lexbor_serialize_write(cb, str_auto.data, str_auto.length,
                                   ctx, status);
            break;

        case LXB_CSS_VALUE_MIN_CONTENT:
            lexbor_serialize_write(cb, str_min.data, str_min.length,
                                   ctx, status);
            break;

        case LXB_CSS_VALUE_MAX_CONTENT:
            lexbor_serialize_write(cb, str_max.data, str_max.length,
                                   ctx, status);
            break;

        case LXB_CSS_VALUE__LENGTH:
            num = &width->u.number;

            /* FIXME: If length != sizeof(buf)? */
            length = lexbor_conv_float_to_data(num->num, buf, sizeof(buf));

            lexbor_serialize_write(cb, buf, length, ctx, status);

            if (num->unit == LXB_CSS_UNIT__UNDEF) {
                break;
            }

            unit = lxb_css_unit_by_id(num->unit);
            if (unit == NULL) {
                break;
            }

            lexbor_serialize_write(cb, unit->name, unit->length, ctx, status);
            break;

        case LXB_CSS_VALUE__PERCENTAGE:
            num = &width->u.number;

            /* FIXME: If length != sizeof(buf)? */
            length = lexbor_conv_float_to_data(num->num, buf, sizeof(buf));

            lexbor_serialize_write(cb, buf, length, ctx, status);
            lexbor_serialize_write(cb, str_per.data, str_per.length,
                                   ctx, status);
            break;

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
