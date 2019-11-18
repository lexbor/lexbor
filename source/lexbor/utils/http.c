/*
* Copyright (C) 2019 Alexander Borisov
*
* Author: Alexander Borisov <borisov@lexbor.com>
*/

#include "lexbor/utils/http.h"

#include "lexbor/core/conv.h"


#ifndef LXB_UTILS_HTTP_MAX_HEADER_FIELD
    #define LXB_UTILS_HTTP_MAX_HEADER_FIELD 4096 * 32
#endif


enum {
    LXB_UTILS_HTTP_STATE_HEAD_VERSION = 0x00,
    LXB_UTILS_HTTP_STATE_HEAD_FIELD,
    LXB_UTILS_HTTP_STATE_HEAD_FIELD_WS,
    LXB_UTILS_HTTP_STATE_HEAD_END,
    LXB_UTILS_HTTP_STATE_BODY,
    LXB_UTILS_HTTP_STATE_BODY_END
};


lxb_inline lxb_status_t
lxb_utils_http_split_field(lxb_utils_http_t *http, const lexbor_str_t *str)
{
    lxb_char_t *p, *end;
    lxb_utils_http_field_t *field;

    field = lexbor_array_obj_push(http->fields);
    if (field == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    p = memchr(str->data, ':', str->length);

    if (p == NULL) {
        http->error = "Wrong header field format.";

        (void) lexbor_array_obj_pop(http->fields);

        return LXB_STATUS_ABORTED;
    }

    field->name.data = str->data;
    field->name.length = p - str->data;

    if (field->name.length == 0) {
        (void) lexbor_array_obj_pop(http->fields);

        return LXB_STATUS_OK;
    }

    p++;
    end = str->data + str->length;

    /* Skip whitespaces before */
    for (; p < end; p++) {
        if (*p != ' ' && *p != '\t') {
            break;
        }
    }

    /* Skip whitespaces after */
    while (end > p) {
        end--;

        if (*end != ' ' && *end != '\t') {
            end++;
            break;
        }
    }

    field->value.data = p;
    field->value.length = end - p;

    return LXB_STATUS_OK;
}

lxb_utils_http_t *
lxb_utils_http_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_utils_http_t));
}

lxb_status_t
lxb_utils_http_init(lxb_utils_http_t *http, lexbor_mraw_t *mraw)
{
    lxb_status_t status;

    if (http == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (mraw == NULL) {
        mraw = lexbor_mraw_create();
        status = lexbor_mraw_init(mraw, 4096 * 4);
        if (status) {
            return status;
        }
    }

    http->mraw = mraw;

    http->fields = lexbor_array_obj_create();
    status = lexbor_array_obj_init(http->fields, 32,
                                   sizeof(lxb_utils_http_field_t));
    if (status) {
        return status;
    }

    lexbor_str_init(&http->tmp, http->mraw, 64);
    if (http->tmp.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    lexbor_str_init(&http->version.name, http->mraw, 8);
    if (http->version.name.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    http->error = NULL;
    http->state = LXB_UTILS_HTTP_STATE_HEAD_VERSION;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_utils_http_clear(lxb_utils_http_t *http)
{
    lexbor_mraw_clean(http->mraw);
    lexbor_array_obj_clean(http->fields);
    lexbor_str_clean_all(&http->tmp);

    http->tmp.data = NULL;

    lexbor_str_init(&http->tmp, http->mraw, 64);
    if (http->tmp.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    http->version.name.data = NULL;
    http->version.number = 0;

    lexbor_str_init(&http->version.name, http->mraw, 8);
    if (http->version.name.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    http->error = NULL;
    http->state = LXB_UTILS_HTTP_STATE_HEAD_VERSION;

    return LXB_STATUS_OK;
}

lxb_utils_http_t *
lxb_utils_http_destroy(lxb_utils_http_t *http, bool self_destroy)
{
    if (http == NULL) {
        return NULL;
    }

    http->mraw = lexbor_mraw_destroy(http->mraw, true);
    http->fields = lexbor_array_obj_destroy(http->fields, true);

    if (self_destroy) {
        return lexbor_free(http);
    }

    return http;
}

lxb_status_t
lxb_utils_http_header_parse_eof(lxb_utils_http_t *http)
{
    if (http->state != LXB_UTILS_HTTP_STATE_HEAD_END) {
        http->error = "Unexpected data termination.";

        return LXB_STATUS_ABORTED;
    }

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_utils_http_parse_version(lxb_utils_http_t *http, const lxb_char_t **data,
                             const lxb_char_t *end)
{
    lexbor_str_t *str;
    const lxb_char_t *p;

    str = &http->version.name;

    p = memchr(*data, '\n', (end - *data));

    if (p == NULL) {
        p = lexbor_str_append(str, http->mraw, *data, (end - *data));
        if (p == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        *data = end;

        if (str->length > LXB_UTILS_HTTP_MAX_HEADER_FIELD) {
            goto to_large;
        }

        return LXB_STATUS_OK;
    }

    *data = lexbor_str_append(str, http->mraw, *data, (p - *data));
    if (*data == NULL) {
        *data = p;
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    *data = p + 1;

    if (str->length < 9 || str->data[(str->length - 1)] != '\r') {
        goto failed;
    }

    (void) lexbor_str_length_set(str, http->mraw, (str->length - 1));

    if (str->length > LXB_UTILS_HTTP_MAX_HEADER_FIELD) {
        goto to_large;
    }

    if (lexbor_str_data_ncasecmp(str->data,
                                 (const lxb_char_t *) "HTTP/", 5) == false)
    {
        goto failed;
    }

    /* Skip version */
    p = str->data + 5;

    http->version.number = lexbor_conv_data_to_double(&p, 3);
    if (http->version.number < 1.0 || http->version.number > 1.1) {
        goto failed;
    }

    /* Skip version */
    end = str->data + str->length;

    if (p != end) {
        if (*p != ' ' && *p != '\t') {
            goto failed;
        }

        /* Skip space */
        for (; p < end; p++) {
            if (*p != ' ' && *p != '\t') {
                break;
            }
        }

        http->version.status = lexbor_conv_data_to_uint(&p, end - p);
        if (http->version.status < 100 || http->version.status >= 600) {
            goto failed;
        }
    }

    http->state = LXB_UTILS_HTTP_STATE_HEAD_FIELD;
    http->tmp.length = 0;

    return LXB_STATUS_OK;

to_large:

    http->error = "Too large header version field.";

    return LXB_STATUS_ABORTED;

failed:

    http->error = "Wrong HTTP version.";

    return LXB_STATUS_ABORTED;
}

lxb_inline lxb_status_t
lxb_utils_http_parse_field(lxb_utils_http_t *http, const lxb_char_t **data,
                           const lxb_char_t *end)
{
    lexbor_str_t *str;
    const lxb_char_t *p;

    str = &http->tmp;

    p = memchr(*data, '\n', (end - *data));

    if (p == NULL) {
        p = lexbor_str_append(str, http->mraw, *data, (end - *data));
        if (p == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        *data = end;

        if (str->length > LXB_UTILS_HTTP_MAX_HEADER_FIELD) {
            goto to_large;
        }

        return LXB_STATUS_OK;
    }

    *data = lexbor_str_append(str, http->mraw, *data, (p - *data));
    if (*data == NULL) {
        *data = p;
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    *data = p + 1;

    if (str->length == 0 || str->data[(str->length - 1)] != '\r') {
        goto failed;
    }

    (void) lexbor_str_length_set(str, http->mraw, (str->length - 1));

    /* Check for end of header */
    if (str->length != 0) {
        if (str->length > LXB_UTILS_HTTP_MAX_HEADER_FIELD) {
            goto to_large;
        }

        http->state = LXB_UTILS_HTTP_STATE_HEAD_FIELD_WS;

        return LXB_STATUS_OK;
    }

    http->state = LXB_UTILS_HTTP_STATE_HEAD_END;

    return LXB_STATUS_OK;

to_large:

    http->error = "Too large header field.";

    return LXB_STATUS_ABORTED;

failed:

    http->error = "Wrong HTTP header filed.";

    return LXB_STATUS_ABORTED;
}

lxb_inline lxb_status_t
lxb_utils_http_parse_field_ws(lxb_utils_http_t *http, const lxb_char_t **data,
                              const lxb_char_t *end)
{
    lxb_status_t status;
    const lxb_char_t *p = *data;

    if (*p != ' ' && *p != '\t') {
        status = lxb_utils_http_split_field(http, &http->tmp);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        lexbor_str_clean_all(&http->tmp);

        lexbor_str_init(&http->tmp, http->mraw, 64);
        if (http->tmp.data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        http->state = LXB_UTILS_HTTP_STATE_HEAD_FIELD;

        return LXB_STATUS_OK;
    }

    for (; p < end; p++) {
        if (*p != ' ' && *p != '\t') {
            *data = p;

            http->state = LXB_UTILS_HTTP_STATE_HEAD_FIELD;

            return LXB_STATUS_OK;
        }
    }

    *data = p;

    return LXB_STATUS_OK;
}

/*
lxb_inline lxb_status_t
lxb_utils_http_parse_body(lxb_utils_http_t *http, const lxb_char_t **data,
                          const lxb_char_t *end)
{
    return LXB_STATUS_OK;
}
*/

lxb_inline lxb_status_t
lxb_utils_http_parse_body_end(lxb_utils_http_t *http,
                              const lxb_char_t **data, const lxb_char_t *end)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_utils_http_parse(lxb_utils_http_t *http,
                     const lxb_char_t **data, const lxb_char_t *end)
{
    lxb_status_t status = LXB_STATUS_ERROR;

    while (*data < end) {
        switch (http->state) {

            case LXB_UTILS_HTTP_STATE_HEAD_VERSION:
                status = lxb_utils_http_parse_version(http, data, end);
                break;

            case LXB_UTILS_HTTP_STATE_HEAD_FIELD:
                status = lxb_utils_http_parse_field(http, data, end);
                break;

            case LXB_UTILS_HTTP_STATE_HEAD_FIELD_WS:
                status = lxb_utils_http_parse_field_ws(http, data, end);
                break;

            case LXB_UTILS_HTTP_STATE_HEAD_END:
                return LXB_STATUS_OK;

            case LXB_UTILS_HTTP_STATE_BODY_END:
                status = lxb_utils_http_parse_body_end(http, data, end);
                break;
        }

        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    /*
     * TODO:
     * We cannot know whether we have a body or not.
     * Need to implementation reading of the body.
     *
     * Please, see Content-Length and Transfer-Encoding with "chunked".
     */
    if (http->state == LXB_UTILS_HTTP_STATE_HEAD_END) {
        return LXB_STATUS_OK;
    }

    return LXB_STATUS_NEXT;
}

lxb_utils_http_field_t *
lxb_utils_http_header_field(lxb_utils_http_t *http, const lxb_char_t *name,
                            size_t len, size_t offset)
{
    lxb_utils_http_field_t *field;

    for (size_t i = 0; i < lexbor_array_obj_length(http->fields); i++) {
        field = lexbor_array_obj_get(http->fields, i);

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
lxb_utils_http_header_serialize(lxb_utils_http_t *http, lexbor_str_t *str)
{
    lxb_status_t status;
    const lxb_utils_http_field_t *field;

    if (str->data == NULL) {
        lexbor_str_init(str, http->mraw, 256);
        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    for (size_t i = 0; i < lexbor_array_obj_length(http->fields); i++) {
        field = lexbor_array_obj_get(http->fields, i);

        status = lxb_utils_http_field_serialize(http, str, field);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_utils_http_field_serialize(lxb_utils_http_t *http, lexbor_str_t *str,
                               const lxb_utils_http_field_t *field)
{
    lxb_char_t *data;

    data = lexbor_str_append(str, http->mraw, field->name.data,
                             field->name.length);
    if (data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = lexbor_str_append(str, http->mraw, (lxb_char_t *) ": ", 2);
    if (data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = lexbor_str_append(str, http->mraw, field->value.data,
                             field->value.length);
    if (data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data = lexbor_str_append_one(str, http->mraw, '\n');
    if (data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}
