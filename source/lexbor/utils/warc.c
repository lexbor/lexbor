/*
* Copyright (C) 2019 Alexander Borisov
*
* Author: Alexander Borisov <borisov@lexbor.com>
*/

#include "lexbor/utils/warc.h"

#include "lexbor/core/fs.h"
#include "lexbor/core/conv.h"


#ifndef LXB_UTILS_WARC_MAX_HEADER_NAME
    #define LXB_UTILS_WARC_MAX_HEADER_NAME 4096 * 4
#endif

#ifndef LXB_UTILS_WARC_MAX_HEADER_VALUE
    #define LXB_UTILS_WARC_MAX_HEADER_VALUE 4096 * 32
#endif


enum {
    LXB_UTILS_WARC_STATE_HEAD_VERSION = 0x00,
    LXB_UTILS_WARC_STATE_HEAD_VERSION_AFTER,
    LXB_UTILS_WARC_STATE_HEAD_FIELD_NAME,
    LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE,
    LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_QUOTED,
    LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_AFTER,
    LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_WS,
    LXB_UTILS_WARC_STATE_HEAD_END,
    LXB_UTILS_WARC_STATE_BLOCK,
    LXB_UTILS_WARC_STATE_BLOCK_AFTER
};

static const lxb_char_t lxb_utils_warc_seporators_ctl[0x80] =
{
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01
};

/*
static const lxb_char_t lxb_utils_warc_ctl[0x80] =
{
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};
*/


lxb_inline lxb_utils_warc_field_t *
lxb_utils_warc_field_append(lxb_utils_warc_t *warc, lxb_char_t *name,
                            size_t len)
{
    lxb_utils_warc_field_t *field;

    field = lexbor_array_obj_push(warc->fields);
    if (field == NULL) {
        return NULL;
    }

    field->name.data = name;
    field->name.length = len;

    return field;
}

lxb_inline lxb_utils_warc_field_t *
lxb_utils_warc_field_last(lxb_utils_warc_t *warc)
{
    return lexbor_array_obj_last(warc->fields);
}


lxb_utils_warc_t *
lxb_utils_warc_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_utils_warc_t));
}

lxb_status_t
lxb_utils_warc_init(lxb_utils_warc_t *warc, lxb_utils_warc_header_cb_f h_cd,
                    lxb_utils_warc_content_cb_f c_cb,
                    lxb_utils_warc_content_end_cb_f c_end_cb, void *ctx)
{
    lxb_status_t status;

    if (warc == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    warc->mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(warc->mraw, 4096 * 4);
    if (status) {
        return status;
    }

    warc->fields = lexbor_array_obj_create();
    status = lexbor_array_obj_init(warc->fields, 32,
                                   sizeof(lxb_utils_warc_field_t));
    if (status) {
        return status;
    }

    lexbor_str_init(&warc->tmp, warc->mraw, 64);
    if (warc->tmp.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    lexbor_str_init(&warc->version.type, warc->mraw, 8);
    if (warc->version.type.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    warc->header_cb = h_cd;
    warc->content_cb = c_cb;
    warc->content_end_cb = c_end_cb;

    warc->error = NULL;
    warc->state = LXB_UTILS_WARC_STATE_HEAD_VERSION;
    warc->count = 0;
    warc->ctx = ctx;
    warc->skip = false;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_utils_warc_clear(lxb_utils_warc_t *warc)
{
    lexbor_mraw_clean(warc->mraw);
    lexbor_array_obj_clean(warc->fields);
    lexbor_str_clean_all(&warc->tmp);

    warc->tmp.data = NULL;

    lexbor_str_init(&warc->tmp, warc->mraw, 64);
    if (warc->tmp.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    warc->version.type.data = NULL;
    warc->version.number = 0;

    lexbor_str_init(&warc->version.type, warc->mraw, 8);
    if (warc->version.type.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    warc->error = NULL;
    warc->state = LXB_UTILS_WARC_STATE_HEAD_VERSION;
    warc->skip = false;

    return LXB_STATUS_OK;
}

lxb_utils_warc_t *
lxb_utils_warc_destroy(lxb_utils_warc_t *warc, bool self_destroy)
{
    if (warc == NULL) {
        return NULL;
    }

    warc->mraw = lexbor_mraw_destroy(warc->mraw, true);
    warc->fields = lexbor_array_obj_destroy(warc->fields, true);

    if (self_destroy) {
        return lexbor_free(warc);
    }

    return warc;
}

lxb_status_t
lxb_utils_warc_parse_file(lxb_utils_warc_t *warc, FILE *fh)
{
    size_t size;
    lxb_status_t status;

    const lxb_char_t *buf_ref;
    lxb_char_t buffer[4096 * 2];

    if (fh == NULL) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    do {
        buf_ref = buffer;

        size = fread(buffer, sizeof(lxb_char_t), sizeof(buffer), fh);
        if (size != sizeof(buffer)) {
            if (feof(fh)) {
                return lxb_utils_warc_parse(warc, &buf_ref, (buffer + size));
            }

            return LXB_STATUS_ERROR;
        }

        status = lxb_utils_warc_parse(warc, &buf_ref,
                                      (buffer + sizeof(buffer)));
    }
    while (status == LXB_STATUS_OK);

    return lxb_utils_warc_parse_eof(warc);
}

lxb_status_t
lxb_utils_warc_parse_eof(lxb_utils_warc_t *warc)
{
    if (warc->state != LXB_UTILS_WARC_STATE_HEAD_VERSION) {
        warc->error = "Unexpected data termination.";

        return LXB_STATUS_ABORTED;
    }

    warc->count = 0;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_version(lxb_utils_warc_t *warc, const lxb_char_t **data,
                             const lxb_char_t *end)
{
    lexbor_str_t *str;
    const lxb_char_t *p;

    str = &warc->version.type;

    p = memchr(*data, '\n', (end - *data));

    if (p == NULL) {
        p = lexbor_str_append(str, warc->mraw, *data, (end - *data));
        if (p == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        *data = end;

        if (str->length > 9) {
            goto failed;
        }

        return LXB_STATUS_OK;
    }

    *data = lexbor_str_append(str, warc->mraw, *data, (p - *data));
    if (*data == NULL) {
        *data = p;
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    *data = p + 1;

    if (str->length < 9 || str->data[(str->length - 1)] != '\r') {
        goto failed;
    }

    lexbor_str_length_set(str, warc->mraw, (str->length - 1));

    if (lexbor_str_data_ncasecmp(str->data,
                                 (const lxb_char_t *) "warc/", 5) == false)
    {
        goto failed;
    }

    p = str->data + 5;

    warc->version.number = lexbor_conv_data_to_double(&p, 3);
    if (warc->version.number != 1.0) {
        goto failed;
    }

    warc->state = LXB_UTILS_WARC_STATE_HEAD_VERSION_AFTER;
    warc->tmp.length = 0;

    return LXB_STATUS_OK;

failed:

    warc->error = "Wrong warc version.";

    return LXB_STATUS_ABORTED;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_field_version_after(lxb_utils_warc_t *warc,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    warc->content_length = 0;

    if (**data != '\r') {
        warc->state = LXB_UTILS_WARC_STATE_HEAD_FIELD_NAME;

        return LXB_STATUS_OK;
    }

    (*data)++;

    warc->state = LXB_UTILS_WARC_STATE_HEAD_END;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_field_name(lxb_utils_warc_t *warc, const lxb_char_t **data,
                          const lxb_char_t *end)
{
    const lxb_char_t *p;
    lxb_utils_warc_field_t *field;

    for (p = *data; p < end; p++) {
        if (*p == ':') {
            *data = lexbor_str_append(&warc->tmp, warc->mraw, *data,
                                      (p - *data));
            if (*data == NULL) {
                *data = p;
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            *data = p + 1;

            field = lxb_utils_warc_field_append(warc, warc->tmp.data,
                                          warc->tmp.length);
            if (field == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            lexbor_str_init(&field->value, warc->mraw, 0);
            if (field->value.data == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            lexbor_str_clean_all(&warc->tmp);

            lexbor_str_init(&warc->tmp, warc->mraw, 64);
            if (warc->tmp.data == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            warc->state = LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_WS;

            return LXB_STATUS_OK;
        }

        if (*p > 0x7F || lxb_utils_warc_seporators_ctl[*p] != 0x00) {
            *data = p;

            warc->error = "Wrong header field name.";

            return LXB_STATUS_ABORTED;
        }
    }

    p = lexbor_str_append(&warc->tmp, warc->mraw, *data, (p - *data));
    if (p == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    if (warc->tmp.length > LXB_UTILS_WARC_MAX_HEADER_NAME) {
        warc->error = "Too large header field name.";

        return LXB_STATUS_ABORTED;
    }

    *data = end;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_field_value(lxb_utils_warc_t *warc,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    const lxb_char_t *p;
    lxb_utils_warc_field_t *field = lxb_utils_warc_field_last(warc);

    for (p = *data; p < end; p++) {
        if (*p == '"') {
            p++;

            *data = lexbor_str_append(&field->value, warc->mraw,
                                      *data, (p - *data));
            if (*data == NULL) {
                *data = p - 1;
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            *data = p;

            warc->state = LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_QUOTED;

            return LXB_STATUS_OK;
        }

        if (*p == '\n') {
            p++;

            *data = lexbor_str_append(&field->value, warc->mraw,
                                      *data, (p - *data));
            if (*data == NULL) {
                *data = p - 1;
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            *data = p;

            if (field->value.length > 1) {
                if (field->value.data[(field->value.length - 2)] == '\r') {

                    lexbor_str_length_set(&field->value, warc->mraw,
                                          (field->value.length - 2));

                    warc->state = LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_AFTER;

                    return LXB_STATUS_OK;
                }
            }

            p--;
        }
/*
        if (*p > 0x7F || lxb_utils_warc_ctl[*p] != 0x00) {
            *data = p;

            warc->error = "Wrong header field value.";

            return LXB_STATUS_ABORTED;
        }
 */
    }

    p = lexbor_str_append(&field->value, warc->mraw, *data, (end - *data));
    if (p == NULL) {
        *data = end;
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    if (field->value.length > LXB_UTILS_WARC_MAX_HEADER_VALUE) {
        warc->error = "Too large header field name.";

        return LXB_STATUS_ABORTED;
    }

    *data = end;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_field_value_quoted(lxb_utils_warc_t *warc,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    const lxb_char_t *p;
    lxb_utils_warc_field_t *field = lxb_utils_warc_field_last(warc);

    for (p = *data; p < end; p++) {
        if (*p == '"') {
            *data = p + 1;

            warc->state = LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE;

            goto done;
        }

        if (*p == '\\') {
            *data = lexbor_str_append(&field->value, warc->mraw,
                                      *data, (p - *data));
            if (*data == NULL) {
                *data = p;
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            *data = ++p;
        }
    }

done:

    p = lexbor_str_append(&field->value, warc->mraw, *data, (end - *data));
    if (p == NULL) {
        *data = end;
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    *data = end;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_field_value_after(lxb_utils_warc_t *warc,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    const lxb_utils_warc_field_t *field = lxb_utils_warc_field_last(warc);
    const lxb_char_t ch = **data;

    static const lxb_char_t lxb_utils_warc_clen[] = "Content-Length";

    if (ch == ' ' || ch == '\t') {
        (*data)++;

        warc->state = LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_WS;

        return LXB_STATUS_OK;
    }

    /* Parse Content-Length value */
    if (warc->content_length == 0
        && field->name.length == (sizeof(lxb_utils_warc_clen) - 1)
        && lexbor_str_data_ncasecmp(field->name.data, lxb_utils_warc_clen,
                                    (sizeof(lxb_utils_warc_clen) - 1)))
    {
        const lxb_char_t *p = field->value.data;
        const lxb_char_t *p_end = p + field->value.length;

        warc->content_length = lexbor_conv_data_to_ulong(&p,
                                                         field->value.length);
        if (p != p_end) {
            warc->error = "Wrong \"Content-Length\" value.";

            return LXB_STATUS_ABORTED;
        }
    }

    if (ch == '\r') {
        (*data)++;

        warc->state = LXB_UTILS_WARC_STATE_HEAD_END;

        return LXB_STATUS_OK;
    }

    warc->state = LXB_UTILS_WARC_STATE_HEAD_FIELD_NAME;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_field_value_ws(lxb_utils_warc_t *warc,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    const lxb_char_t *p;

    for (p = *data; p < end; p++) {
        if (*p != ' ' && *p != '\t') {
            *data = p;

            warc->state = LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE;

            return LXB_STATUS_OK;
        }
    }

    *data = p;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_header_end(lxb_utils_warc_t *warc, const lxb_char_t **data,
                                const lxb_char_t *end)
{
    lxb_status_t status;

    if (**data != '\n') {
        warc->error = "Wrong end of header.";

        return LXB_STATUS_ABORTED;
    }

    (*data)++;

    if (warc->header_cb != NULL) {
        status = warc->header_cb(warc);
        if (status != LXB_STATUS_OK) {
            if (status != LXB_STATUS_NEXT) {
                return status;
            }

            warc->skip = true;
        }
    }

    if (warc->content_length != 0) {
        warc->state = LXB_UTILS_WARC_STATE_BLOCK;
    }
    else {
        warc->state = LXB_UTILS_WARC_STATE_BLOCK_AFTER;
    }

    warc->content_read = 0;
    warc->ends = 0;
    warc->count++;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_block(lxb_utils_warc_t *warc, const lxb_char_t **data,
                           const lxb_char_t *end)
{
    lxb_status_t status = LXB_STATUS_OK;

    if ((end - *data) >= (warc->content_length - warc->content_read)) {
        end = *data + (warc->content_length - warc->content_read);

        if (warc->skip == false && warc->content_cb != NULL) {
            status = warc->content_cb(warc, *data, end);

            if (status != LXB_STATUS_OK) {
                warc->skip = true;
            }
        }

        warc->content_read = warc->content_length;
        warc->state = LXB_UTILS_WARC_STATE_BLOCK_AFTER;
        warc->ends = 0;

        *data = end;

        return status;
    }

    if (warc->skip == false && warc->content_cb != NULL) {
        status = warc->content_cb(warc, *data, end);

        if (status != LXB_STATUS_OK) {
            warc->skip = true;
        }
    }

    warc->content_read += end - *data;
    *data = end;

    return status;
}

lxb_inline lxb_status_t
lxb_utils_warc_parse_block_after(lxb_utils_warc_t *warc,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    lxb_status_t status;
    static const lxb_char_t lxb_utils_warc_ends[] = "\r\n\r\n";

    while (warc->ends < 4) {
        if (**data != lxb_utils_warc_ends[warc->ends]) {
            warc->error = "Wrong end of block.";

            return LXB_STATUS_ERROR;
        }

        warc->ends++;

        if (++(*data) == end) {
            return LXB_STATUS_OK;
        }
    }

    if (warc->skip == false && warc->content_end_cb != NULL) {
        status = warc->content_end_cb(warc);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_utils_warc_clear(warc);
}

lxb_status_t
lxb_utils_warc_parse(lxb_utils_warc_t *warc,
                     const lxb_char_t **data, const lxb_char_t *end)
{
    lxb_status_t status = LXB_STATUS_ERROR;

    while (*data < end) {
        switch (warc->state) {

            case LXB_UTILS_WARC_STATE_HEAD_VERSION:
                status = lxb_utils_warc_parse_version(warc, data, end);
                break;

            case LXB_UTILS_WARC_STATE_HEAD_VERSION_AFTER:
                status = lxb_utils_warc_parse_field_version_after(warc, data, end);
                break;

            case LXB_UTILS_WARC_STATE_HEAD_FIELD_NAME:
                status = lxb_utils_warc_parse_field_name(warc, data, end);
                break;

            case LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE:
                status = lxb_utils_warc_parse_field_value(warc, data, end);
                break;

            case LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_QUOTED:
                status = lxb_utils_warc_parse_field_value_quoted(warc, data, end);
                break;

            case LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_AFTER:
                status = lxb_utils_warc_parse_field_value_after(warc, data, end);
                break;

            case LXB_UTILS_WARC_STATE_HEAD_FIELD_VALUE_WS:
                status = lxb_utils_warc_parse_field_value_ws(warc, data, end);
                break;

            case LXB_UTILS_WARC_STATE_HEAD_END:
                status = lxb_utils_warc_parse_header_end(warc, data, end);
                break;

            case LXB_UTILS_WARC_STATE_BLOCK:
                status = lxb_utils_warc_parse_block(warc, data, end);
                break;

            case LXB_UTILS_WARC_STATE_BLOCK_AFTER:
                status = lxb_utils_warc_parse_block_after(warc, data, end);
                break;
        }

        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

lxb_utils_warc_field_t *
lxb_utils_warc_header_field(lxb_utils_warc_t *warc, const lxb_char_t *name,
                            size_t len, size_t offset)
{
    lxb_utils_warc_field_t *field;

    for (size_t i = 0; i < lexbor_array_obj_length(warc->fields); i++) {
        field = lexbor_array_obj_get(warc->fields, i);

        if (field->name.length == len
            && lexbor_str_data_ncasecmp(field->name.data, name, len))
        {
            if (offset == 0) {
                return field;
            }

            offset--;
        }
    }

    return NULL;
}

lxb_status_t
lxb_utils_warc_header_serialize(lxb_utils_warc_t *warc, lexbor_str_t *str)
{
    lxb_char_t *data;
    const lxb_utils_warc_field_t *field;

    if (str->data == NULL) {
        lexbor_str_init(str, warc->mraw, 256);
        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    for (size_t i = 0; i < lexbor_array_obj_length(warc->fields); i++) {
        field = lexbor_array_obj_get(warc->fields, i);

        data = lexbor_str_append(str, warc->mraw, field->name.data,
                                 field->name.length);
        if (data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        data = lexbor_str_append(str, warc->mraw, (lxb_char_t *) ": ", 2);
        if (data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        data = lexbor_str_append(str, warc->mraw, field->value.data,
                                 field->value.length);
        if (data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        data = lexbor_str_append_one(str, warc->mraw, '\n');
        if (data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    return LXB_STATUS_OK;
}

/*
 * No inline functions for ABI.
 */
size_t
lxb_utils_warc_content_length_noi(lxb_utils_warc_t *warc)
{
    return lxb_utils_warc_content_length(warc);
}
