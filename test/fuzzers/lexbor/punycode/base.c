/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */


#include <lexbor/core/str.h>
#include <lexbor/encoding/encoding.h>
#include <lexbor/punycode/punycode.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx, bool unchanged);

static lxb_status_t
callback_cp(const lxb_codepoint_t *cps, size_t len, void *ctx);


int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t length)
{
    lxb_status_t status;
    lexbor_str_t str;
    const lxb_char_t *input_end;
    lxb_codepoint_t cp, *p, *end;
    lxb_codepoint_t source[4096 * 64];

    input_end = data + length;

    p = source;
    end = p + (sizeof(source) / sizeof(lxb_codepoint_t));

    while (data < input_end) {
        cp = lxb_encoding_decode_valid_utf_8_single(&data, input_end);
        if (cp == LXB_ENCODING_DECODE_ERROR) {
            cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
        }

        *p++ = cp;

        if (p >= end) {
            break;
        }
    }

    str.data = NULL;

    status = lxb_punycode_encode_cp(source, p - source, callback, &str);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    (void) lxb_punycode_decode_cb_cp(str.data, str.length, callback_cp, NULL);

done:

    if (str.data != NULL) {
        lexbor_free(str.data);
    }

    return EXIT_SUCCESS;
}


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx, bool unchanged)
{
    lexbor_str_t *str = ctx;

    str->data = lexbor_malloc(len + 1);
    if (str->data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    str->length = len;

    memcpy(str->data, data, len);

    str->data[len] = '\0';

    return LXB_STATUS_OK;
}

static lxb_status_t
callback_cp(const lxb_codepoint_t *cps, size_t len, void *ctx)
{
    return LXB_STATUS_OK;
}
