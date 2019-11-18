/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "../parser.h"
#include "lexbor/encoding/multi.h"
#include "lexbor/encoding/range.h"


#define to_update_buffer(data)                                                 \
    buf = (lxb_char_t *) data;                                                 \
    end = buf + strlen((const char *) data)

#define to_update_buffer_size(data, size)                                      \
    buf = (lxb_char_t *) data;                                                 \
    end = buf + size

#define to_cps(count, ...) (lxb_codepoint_t [count]) {__VA_ARGS__}

#define test_buffer(func, count, ...)                                          \
    do {                                                                       \
        size = func(enc_data, buf, end, cps_buffer,                            \
             cps_buffer + (sizeof(cps_buffer) / sizeof(lxb_codepoint_t)));     \
        if (size != count) {                                                   \
            test_call_error();                                                 \
        }                                                                      \
                                                                               \
        test_eq(memcmp(to_cps(count, __VA_ARGS__),                             \
                       cps_buffer, sizeof(lxb_codepoint_t) * count), 0);       \
    }                                                                          \
    while (0)


void
failed_and_exit(size_t line)
{
    printf("Failed to parse file; line: "LEXBOR_FORMAT_Z"\n", line);

    exit(EXIT_FAILURE);
}

static size_t
test_decode_chunks(const lxb_encoding_data_t *enc_data,
                   const lxb_char_t *buf, const lxb_char_t *end,
                   lxb_codepoint_t *cps, lxb_codepoint_t *cps_end)
{
    lxb_char_t ch;
    lxb_status_t status;
    lxb_codepoint_t cp, *begin;
    const lxb_char_t *ch_ref, *ch_end;
    lxb_encoding_decode_t ctx;

    ch_ref = &ch;
    ch_end = ch_ref + 1;
    begin = cps;

    status = lxb_encoding_decode_init_single(&ctx, enc_data);
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    /* Imitate the work with chunks */
    while (buf < end) {
        ch = *buf;

        cp = enc_data->decode_single(&ctx, &ch_ref, ch_end);

        if (ch_ref == ch_end) {
            ch_ref--;
            buf++;
        }

        if (cp > LXB_ENCODING_DECODE_MAX_CODEPOINT) {
            /* Need to save last code point == CONTINUE */
            if (cp == LXB_ENCODING_DECODE_CONTINUE) {
                if (buf < end) {
                    continue;
                }
            }
            else {
                cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
            }
        }

        *cps++ = cp;
        if (cps >= cps_end) {
            return 0;
        }
    }

    return cps - begin;
}

static size_t
test_decode_full(const lxb_encoding_data_t *enc_data,
                 const lxb_char_t *buf, const lxb_char_t *end,
                 lxb_codepoint_t *cps, lxb_codepoint_t *cps_end)
{
    lxb_status_t status;
    lxb_codepoint_t cp, *begin;
    lxb_encoding_decode_t ctx;

    status = lxb_encoding_decode_init_single(&ctx, enc_data);
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    begin = cps;

    while (buf < end) {
        cp = enc_data->decode_single(&ctx, &buf, end);

        if (cp > LXB_ENCODING_DECODE_MAX_CODEPOINT) {
            if (cp == LXB_ENCODING_DECODE_CONTINUE) {
                if (buf < end) {
                    continue;
                }
            }
            else {
                cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
            }
        }

        *cps++ = cp;
        if (cps >= cps_end) {
            return 0;
        }
    }

    return cps - begin;
}

static lxb_status_t
test_encode_process_file(const lxb_test_entry_t *entry, void *ctx, size_t line)
{
    int8_t size, j;
    lxb_char_t data[16];
    lxb_char_t *ref, *end;
    lxb_status_t status;
    lxb_encoding_encode_t enc_ctx;
    const lxb_encoding_data_t *enc_data = ctx;

    status = lxb_encoding_encode_init_single(&enc_ctx, enc_data);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    ref = data;
    end = data + sizeof(data) - 1;

    size = enc_data->encode_single(&enc_ctx, &ref, end, entry->cp);

    if (enc_data->encoding == LXB_ENCODING_ISO_2022_JP) {
        size += lxb_encoding_encode_iso_2022_jp_eof_single(&enc_ctx, &ref, end);
    }

    if (size != entry->size
        || strncmp((const char *) data, (const char *) entry->data, size))
    {
        TEST_PRINT("Need: ");
        for (j = 0; j < (int8_t) entry->size; j++) {
            TEST_PRINT("\\x%02X", entry->data[j]);
        }

        TEST_PRINT("\n");
        TEST_PRINT("Have: ");

        if (size < LXB_ENCODING_ENCODE_OK) {
            TEST_PRINT("<ERROR>");
        }
        else {
            for (j = 0; j < size; j++) {
                TEST_PRINT("\\x%02X", data[j]);
            }
        }

        TEST_PRINT("\n");

        TEST_PRINTLN("Codepoint: %u; Line: "LEXBOR_FORMAT_Z, entry->cp, line);

        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
test_decode_process_file(const lxb_test_entry_t *entry, void *ctx, size_t line)
{
    lxb_status_t status;

    const lxb_char_t *data, *end;
    lxb_codepoint_t cp;

    lxb_encoding_decode_t enc_ctx;
    const lxb_encoding_data_t *enc_data = ctx;

    status = lxb_encoding_decode_init_single(&enc_ctx, enc_data);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    data = entry->data;
    end = data + entry->size;

    cp = enc_data->decode_single(&enc_ctx, &data, end);
    if (cp != entry->cp) {
        TEST_PRINTLN("Need: %u; Have: %u; Line: "LEXBOR_FORMAT_Z,
                     entry->cp, cp, line);

        TEST_PRINT("Bytes: ");
        for (size_t j = 0; j < entry->size; j++) {
            TEST_PRINT("\\x%02X", entry->data[j]);
        }

        TEST_PRINT("\n");

        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}
