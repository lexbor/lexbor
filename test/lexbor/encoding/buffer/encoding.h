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


enum {
    LXB_TEST_ENCODING_CONTINUE_CODEPOINT = 0xFFFFFF
};


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
    const lxb_codepoint_t *buf_cont;
    const lxb_char_t *ch_ref, *ch_end;
    lxb_encoding_decode_t dectx;

    buf_cont = &((const lxb_codepoint_t) {LXB_TEST_ENCODING_CONTINUE_CODEPOINT});

    ch_ref = &ch;
    ch_end = ch_ref + 1;

    status = lxb_encoding_decode_init(&dectx, enc_data, cps, (cps_end - cps));
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    status = lxb_encoding_decode_replace_set(&dectx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    /* Imitate the work with chunks */
    while (buf < end) {
        ch = *buf;
        ch_ref = &ch;

        status = enc_data->decode(&dectx, &ch_ref, ch_end);

        if (status != LXB_STATUS_OK) {
            if (status == LXB_STATUS_SMALL_BUFFER) {
                return lxb_encoding_decode_buf_used(&dectx);
            }

            if (status == LXB_STATUS_CONTINUE) {
                if (++buf >= end) {
                    lxb_encoding_decode_buf_add_to(&dectx, buf_cont, 1);
                    break;
                }

                continue;
            }
        }

        buf++;
    }

    return lxb_encoding_decode_buf_used(&dectx);
}

static size_t
test_decode_full(const lxb_encoding_data_t *enc_data,
                 const lxb_char_t *buf, const lxb_char_t *end,
                 lxb_codepoint_t *cps, lxb_codepoint_t *cps_end)
{
    lxb_status_t status;
    lxb_codepoint_t *buf_cont;
    lxb_encoding_decode_t dectx;

    buf_cont = &(lxb_codepoint_t) {LXB_TEST_ENCODING_CONTINUE_CODEPOINT};

    status = lxb_encoding_decode_init(&dectx, enc_data, cps, (cps_end - cps));
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    status = lxb_encoding_decode_replace_set(&dectx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    status = enc_data->decode(&dectx, &buf, end);

    if (status == LXB_STATUS_CONTINUE) {
        lxb_encoding_decode_buf_add_to(&dectx, buf_cont, 1);
    }

    return lxb_encoding_decode_buf_used(&dectx);
}

static lxb_status_t
test_encode_process_file(const lxb_test_entry_t *entry, void *ctx, size_t line)
{
    lxb_status_t status;
    lxb_encoding_encode_t enctx;
    const lxb_encoding_data_t *enc_data = ctx;

    const lxb_codepoint_t *cps = &entry->cp;
    const lxb_codepoint_t *cps_end = cps + 1;

    lxb_char_t data[16];

    status = lxb_encoding_encode_init(&enctx, enc_data, data, sizeof(data));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = lxb_encoding_encode_replace_set(&enctx,
                 LXB_ENCODING_REPLACEMENT_BYTES, LXB_ENCODING_REPLACEMENT_SIZE);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = enc_data->encode(&enctx, &cps, cps_end);

    if (status == LXB_STATUS_OK) {
        status = lxb_encoding_encode_finish(&enctx);
    }

    if (status != LXB_STATUS_OK
        || lxb_encoding_encode_buf_used(&enctx) != entry->size
        || strncmp((const char *) data, (const char *) entry->data, entry->size))
    {
        TEST_PRINT("Need: ");
        for (size_t i = 0; i < entry->size; i++) {
            TEST_PRINT("\\x%02X", entry->data[i]);
        }

        TEST_PRINT("\n");
        TEST_PRINT("Have: ");

        for (size_t i = 0; i < lxb_encoding_encode_buf_used(&enctx); i++) {
            TEST_PRINT("\\x%02X", data[i]);
        }

        TEST_PRINT("\n");

        TEST_PRINTLN("Codepoint: %u; Line: "LEXBOR_FORMAT_Z, entry->cp, line);

        return LXB_STATUS_ERROR;
    }

    return status;
}

static lxb_status_t
test_decode_process_file(const lxb_test_entry_t *entry, void *ctx, size_t line)
{
    lxb_status_t status;
    lxb_encoding_decode_t dectx;
    const lxb_encoding_data_t *enc_data = ctx;

    lxb_codepoint_t buf[16];
    const lxb_char_t *data, *end;

    data = entry->data;
    end = data + entry->size;

    status = lxb_encoding_decode_init(&dectx, enc_data, buf,
                                      sizeof(buf) / sizeof(lxb_codepoint_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = lxb_encoding_decode_replace_set(&dectx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* No need to check status. Status can be CONTINUE or OK */
    enc_data->decode(&dectx, &data, end);

    status = lxb_encoding_decode_finish(&dectx);

    if (lxb_encoding_decode_buf_used(&dectx) != 1
        || buf[0] != entry->cp)
    {
        if (status == LXB_STATUS_SMALL_BUFFER) {
            TEST_PRINTLN("Small buffer;");
        }
        else if (status == LXB_STATUS_CONTINUE) {
            TEST_PRINTLN("Have continue;");
        }

        TEST_PRINTLN(" Need: %u; Line: "LEXBOR_FORMAT_Z, entry->cp, line);

        TEST_PRINT("Bytes: ");
        for (size_t i = 0; i < entry->size; i++) {
            TEST_PRINT("\\x%02X", entry->data[i]);
        }

        TEST_PRINT("\n");

        return LXB_STATUS_ERROR;
    }

    return status;
}
