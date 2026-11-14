/*
 * Copyright (C) 2019-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include "encoding.h"


static const char *lxb_filepath_test;


typedef struct {
    const lxb_codepoint_t *source;
    const lxb_char_t      *result;
}
test_sequence;


static size_t
test_decode_finish(const lxb_encoding_data_t *enc_data,
                   const lxb_char_t *buf, const lxb_char_t *end,
                   lxb_codepoint_t *cps, lxb_codepoint_t *cps_end)
{
    lxb_status_t status;
    lxb_encoding_decode_t dectx;

    status = lxb_encoding_decode_init(&dectx, enc_data, cps, (cps_end - cps));
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    status = lxb_encoding_decode_replace_set(&dectx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    (void) enc_data->decode(&dectx, &buf, end);

    status = lxb_encoding_decode_finish(&dectx);
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    return lxb_encoding_decode_buf_used(&dectx);
}

TEST_BEGIN(decode)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);
    test_ne(enc_data, NULL);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x1B\x24\x40\x1B\x24\x40");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x1B\x23");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x23);
    test_buffer(test_decode_full, 2, rp_cp, 0x23);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x1B\x24\x40\x21\x1B");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x1B\x24\x40\x21\x10\x21\x21");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_chunks, 1, 0x3000);
    test_buffer(test_decode_full, 1, 0x3000);

    to_update_buffer("\x1B\x24\x40\x20\x21\x21");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x24\x40\x21\x20\x21\x21");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x24\x40\x7E\x21");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x1B\x24\x40\x7E\x7E");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x1B\x23\x32");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x23, 0x32);
    test_buffer(test_decode_full, 3, rp_cp, 0x23, 0x32);

    to_update_buffer("\x1B\x28\x42\x32");
    test_buffer(test_decode_chunks, 1, 0x32);
    test_buffer(test_decode_full, 1, 0x32);

    /* ASCII */
    to_update_buffer("\x32");
    test_buffer(test_decode_chunks, 1, 0x32);
    test_buffer(test_decode_full, 1, 0x32);

    to_update_buffer("\x1B\x28\x42\x0E");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x42\x0F");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x42\x0F\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_chunks, 3, rp_cp, rp_cp, 0x3000);
    test_buffer(test_decode_full, 3, rp_cp, rp_cp, 0x3000);

    /* Roman */
    to_update_buffer("\x1B\x28\x4A\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x4A\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_chunks, 3, rp_cp, rp_cp, 0x3000);
    test_buffer(test_decode_full, 3, rp_cp, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x4A\x5C\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_chunks, 3, 0xA5, rp_cp, 0x3000);
    test_buffer(test_decode_full, 3, 0xA5, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x4A\x5C");
    test_buffer(test_decode_chunks, 1, 0xA5);
    test_buffer(test_decode_full, 1, 0xA5);

    to_update_buffer("\x1B\x28\x4A\x7E");
    test_buffer(test_decode_chunks, 1, 0x203E);
    test_buffer(test_decode_full, 1, 0x203E);

    to_update_buffer("\x1B\x28\x4A\x32\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_chunks, 3, 0x32, rp_cp, 0x3000);
    test_buffer(test_decode_full, 3, 0x32, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x4A\x32");
    test_buffer(test_decode_chunks, 1, 0x32);
    test_buffer(test_decode_full, 1, 0x32);

    to_update_buffer("\x1B\x28\x4A\x0E");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x4A\x0F");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x4A\x0F\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_chunks, 3, rp_cp, rp_cp, 0x3000);
    test_buffer(test_decode_full, 3, rp_cp, rp_cp, 0x3000);

    /* katakana */
    to_update_buffer("\x1B\x28\x49\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x49\x21");
    test_buffer(test_decode_chunks, 1, 0xFF61);
    test_buffer(test_decode_full, 1, 0xFF61);

    to_update_buffer("\x1B\x28\x49\x20");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x49\x5F");
    test_buffer(test_decode_chunks, 1, 0xFF9F);
    test_buffer(test_decode_full, 1, 0xFF9F);

    to_update_buffer("\x1B\x28\x49\x60");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x49\x0F\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_chunks, 3, rp_cp, rp_cp, 0x3000);
    test_buffer(test_decode_full, 3, rp_cp, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x49\x5F\x1B\x28\x49\x5F");
    test_buffer(test_decode_chunks, 2, 0xFF9F, 0xFF9F);
    test_buffer(test_decode_full, 2, 0xFF9F, 0xFF9F);
}
TEST_END

/* Broken encoding. Prepend to stream test. */
TEST_BEGIN(decode_prepend)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);
    test_ne(enc_data, NULL);

    to_update_buffer("\x1B\x26\x32");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x26, 0x32);
    test_buffer(test_decode_full, 3, rp_cp, 0x26, 0x32);

    to_update_buffer("\x1B\x24\x32");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x24, 0x32);
    test_buffer(test_decode_full, 3, rp_cp, 0x24, 0x32);
}
TEST_END

TEST_BEGIN(decode_buffer_check_escape_dollar)
{
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;
    lxb_encoding_decode_t dectx;
    const lxb_char_t *data;
    lxb_codepoint_t resume[4];
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;

    static const lxb_char_t invalid_escape[] = {0x1B, 0x24, 0x32};

    struct {
        lxb_codepoint_t out[1];
        lxb_codepoint_t canary;
    } storage = {{0}, 0xA5A5A5A5};

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);
    test_ne(enc_data, NULL);

    data = invalid_escape;

    status = lxb_encoding_decode_init(&dectx, enc_data, storage.out, 1);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_encoding_decode_replace_set(&dectx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    test_eq(status, LXB_STATUS_OK);

    status = enc_data->decode(&dectx, &data,
                              invalid_escape + sizeof(invalid_escape));
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(storage.out[0], rp_cp);
    test_eq(storage.canary, 0xA5A5A5A5);
    test_eq_size((size_t) (data - invalid_escape), 2);

    memset(resume, 0, sizeof(resume));
    lxb_encoding_decode_buf_set(&dectx, resume,
                                sizeof(resume) / sizeof(lxb_codepoint_t));

    status = enc_data->decode(&dectx, &data,
                              invalid_escape + sizeof(invalid_escape));
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 2);
    test_eq(resume[0], 0x24);
    test_eq(resume[1], 0x32);
}
TEST_END

TEST_BEGIN(decode_buffer_check_escape_paren)
{
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;
    lxb_encoding_decode_t dectx;
    const lxb_char_t *data;
    lxb_codepoint_t resume[4];
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;

    static const lxb_char_t invalid_escape[] = {0x1B, 0x28, 0x32};

    struct {
        lxb_codepoint_t out[1];
        lxb_codepoint_t canary;
    } storage = {{0}, 0xA5A5A5A5};

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);
    test_ne(enc_data, NULL);

    data = invalid_escape;

    status = lxb_encoding_decode_init(&dectx, enc_data, storage.out, 1);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_encoding_decode_replace_set(&dectx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    test_eq(status, LXB_STATUS_OK);

    status = enc_data->decode(&dectx, &data,
                              invalid_escape + sizeof(invalid_escape));
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(storage.out[0], rp_cp);
    test_eq(storage.canary, 0xA5A5A5A5);
    test_eq_size((size_t) (data - invalid_escape), 2);

    memset(resume, 0, sizeof(resume));
    lxb_encoding_decode_buf_set(&dectx, resume,
                                sizeof(resume) / sizeof(lxb_codepoint_t));

    status = enc_data->decode(&dectx, &data,
                              invalid_escape + sizeof(invalid_escape));
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 2);
    test_eq(resume[0], 0x28);
    test_eq(resume[1], 0x32);
}
TEST_END

TEST_BEGIN(decode_buffer_check_escape_start)
{
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;
    lxb_encoding_decode_t dectx;
    const lxb_char_t *data;
    lxb_codepoint_t out[2];

    static const lxb_codepoint_t replace[] = {0xFFFD, 0x3F};
    static const lxb_char_t invalid_escape[] = {0x41, 0x1B, 0x42};

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    data = invalid_escape;

    status = lxb_encoding_decode_init(&dectx, enc_data, out, 2);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_encoding_decode_replace_set(&dectx, replace, 2);
    test_eq(status, LXB_STATUS_OK);

    status = enc_data->decode(&dectx, &data,
                              invalid_escape + sizeof(invalid_escape));
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(out[0], 0x41);

    lxb_encoding_decode_buf_used_set(&dectx, 0);

    status = enc_data->decode(&dectx, &data,
                              invalid_escape + sizeof(invalid_escape));
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 2);
    test_eq(out[0], 0xFFFD);
    test_eq(out[1], 0x3F);

    lxb_encoding_decode_buf_used_set(&dectx, 0);

    status = enc_data->decode(&dectx, &data,
                              invalid_escape + sizeof(invalid_escape));
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(out[0], 0x42);
}
TEST_END

TEST_BEGIN(decode_buffer_check_trail)
{
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;
    lxb_encoding_decode_t dectx;
    const lxb_char_t *data;
    lxb_codepoint_t out[2];

    static const lxb_codepoint_t replace[] = {0xFFFD, 0x3F};
    static const lxb_char_t invalid_trail[] = {0x41, 0x1B, 0x24, 0x42,
                                               0x30, 0x0A, 0x30, 0x21};

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    data = invalid_trail;

    status = lxb_encoding_decode_init(&dectx, enc_data, out, 2);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_encoding_decode_replace_set(&dectx, replace, 2);
    test_eq(status, LXB_STATUS_OK);

    status = enc_data->decode(&dectx, &data,
                              invalid_trail + sizeof(invalid_trail));
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(out[0], 0x41);

    lxb_encoding_decode_buf_used_set(&dectx, 0);

    status = enc_data->decode(&dectx, &data,
                              invalid_trail + sizeof(invalid_trail));
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 2);
    test_eq(out[0], 0xFFFD);
    test_eq(out[1], 0x3F);

    lxb_encoding_decode_buf_used_set(&dectx, 0);

    status = enc_data->decode(&dectx, &data,
                              invalid_trail + sizeof(invalid_trail));
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(out[0], 0x4E9C);
}
TEST_END

TEST_BEGIN(decode_finish_after_escape)
{
    size_t i;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;
    lxb_encoding_decode_t dectx;
    const lxb_char_t *data;
    lxb_codepoint_t out[4];

    static const lxb_char_t escapes[][3] = {
        {0x1B, 0x28, 0x42}, {0x1B, 0x28, 0x4A}, {0x1B, 0x28, 0x49},
        {0x1B, 0x24, 0x40}, {0x1B, 0x24, 0x42}
    };

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    for (i = 0; i < sizeof(escapes) / sizeof(escapes[0]); i++) {
        data = escapes[i];

        status = lxb_encoding_decode_init(&dectx, enc_data, out, 4);
        test_eq(status, LXB_STATUS_OK);

        status = lxb_encoding_decode_replace_set(&dectx,
              LXB_ENCODING_REPLACEMENT_BUFFER,
              LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
        test_eq(status, LXB_STATUS_OK);

        (void) enc_data->decode(&dectx, &data, escapes[i] + 3);

        status = lxb_encoding_decode_finish(&dectx);
        test_eq(status, LXB_STATUS_OK);
        test_eq_size(lxb_encoding_decode_buf_used(&dectx), 0);
    }
}
TEST_END

TEST_BEGIN(decode_finish_escape)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[16];

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    to_update_buffer("\x1B");
    test_buffer(test_decode_finish, 1, rp_cp);

    to_update_buffer("\x1B\x24");
    test_buffer(test_decode_finish, 2, rp_cp, 0x24);

    to_update_buffer("\x1B\x28");
    test_buffer(test_decode_finish, 2, rp_cp, 0x28);

    to_update_buffer("\x1B\x28\x4A\x1B\x24");
    test_buffer(test_decode_finish, 2, rp_cp, 0x24);

    to_update_buffer("\x1B\x28\x49\x1B\x28");
    test_buffer(test_decode_finish, 2, rp_cp, 0xFF68);

    to_update_buffer("\x1B\x24\x42\x1B\x24");
    test_buffer(test_decode_finish, 2, rp_cp, rp_cp);

    to_update_buffer("\x1B\x24\x42\x30");
    test_buffer(test_decode_finish, 1, rp_cp);
}
TEST_END

TEST_BEGIN(decode_buffer_check_finish_escape)
{
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;
    lxb_encoding_decode_t dectx;
    const lxb_char_t *data;
    lxb_codepoint_t rp_cp;
    lxb_codepoint_t out[2];

    static const lxb_char_t escape[] = {0x41, 0x1B, 0x24};

    rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    data = escape;

    status = lxb_encoding_decode_init(&dectx, enc_data, out, 2);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_encoding_decode_replace_set(&dectx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    test_eq(status, LXB_STATUS_OK);

    status = enc_data->decode(&dectx, &data, escape + sizeof(escape));
    test_eq(status, LXB_STATUS_CONTINUE);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);

    status = lxb_encoding_decode_finish(&dectx);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 2);
    test_eq(out[0], 0x41);
    test_eq(out[1], rp_cp);

    lxb_encoding_decode_buf_used_set(&dectx, 0);

    status = lxb_encoding_decode_finish(&dectx);
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(out[0], 0x24);

    lxb_encoding_decode_buf_used_set(&dectx, 0);

    status = lxb_encoding_decode_finish(&dectx);
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 0);
}
TEST_END

TEST_BEGIN(decode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    status = test_encoding_process_file(lxb_filepath_test,
                                        test_decode_process_file,
                                        (void *) enc_data, &line);
    if (status != LXB_STATUS_OK) {
        failed_and_exit(line);
    }
}
TEST_END

TEST_BEGIN(encode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    status = test_encoding_process_file(lxb_filepath_test,
                                        test_encode_process_file,
                                        (void *) enc_data, &line);
    if (status != LXB_STATUS_OK) {
        failed_and_exit(line);
    }
}
TEST_END

TEST_BEGIN(encode_buffer_check)
{
    lxb_status_t status;
    lxb_char_t ch1, ch2[2], ch3[3], ch4[4], ch5[5];

    lxb_codepoint_t cp;
    const lxb_codepoint_t *cps;
    lxb_encoding_encode_t enctx;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    lxb_encoding_encode_init(&enctx, enc_data, &ch1, sizeof(ch1));

    /* 4 */
    cp = 0x00A5;

    cps = &cp;
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch2, 2);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch3, 3);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch4, 4);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 4);

    /* 5 */
    cp = 0xFF61;

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, &ch1, 1);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch2, 2);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch3, 3);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch4, 4);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch5, 5);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 5);
}
TEST_END

TEST_BEGIN(encode_sequence)
{
    lxb_status_t status;
    lxb_encoding_encode_t encode;
    const lxb_codepoint_t *src, *src_end;
    const lxb_encoding_data_t *encoding;

    lxb_char_t buffer[1024];
    const lxb_char_t *end = buffer + sizeof(buffer);

    const test_sequence cps[] = {
        {
            .source = (lxb_codepoint_t[]) {0x00A5, 0x203E, 0x00},
            .result = (lxb_char_t[]) {0x1B, 0x28, 0x4A, 0x5C, 0x7E, 0x1B, 0x28, 0x42, 0x00}
        },
        {
            .source = (lxb_codepoint_t[]) {0x203E, 0x00A5, 0x00},
            .result = (lxb_char_t[]) {0x1B, 0x28, 0x4A, 0x7E, 0x5C, 0x1B, 0x28, 0x42, 0x00}
        }
    };

    encoding = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    for (size_t i = 0; i != (sizeof(cps) / sizeof(test_sequence)); i++) {
        src_end = cps[i].source;
        while (*src_end != 0x00) {
            src_end += 1;
        }
        src = cps[i].source;

        status = lxb_encoding_encode_init(&encode, encoding, buffer, end - buffer);
        test_eq(status, LXB_STATUS_OK);

        status = encoding->encode(&encode, &src, src_end);
        test_eq(status, LXB_STATUS_OK);

        status = lxb_encoding_encode_finish(&encode);
        test_eq(status, LXB_STATUS_OK);

        test_eq_str_n(buffer, lxb_encoding_encode_buf_used(&encode),
                      cps[i].result, strlen((const char *) cps[i].result));
    }
}
TEST_END

TEST_BEGIN(encode_sequence_small_buffer)
{
    size_t length, used;
    lxb_status_t status;
    lxb_encoding_encode_t encode;
    const lxb_codepoint_t *cps, *end;
    const lxb_encoding_data_t *encoding;
    lxb_char_t buffer[32], out[64];

    static const lxb_codepoint_t source[] = {
        0x3042, 0x0041, 0x00A5, 0x005C, 0x4E00, 0x007E, 0x203E, 0x3042
    };

    static const lxb_char_t result[] = {
        0x1B, 0x24, 0x42, 0x24, 0x22, 0x1B, 0x28, 0x42, 0x41,
        0x1B, 0x28, 0x4A, 0x5C, 0x1B, 0x28, 0x42, 0x5C,
        0x1B, 0x24, 0x42, 0x30, 0x6C, 0x1B, 0x28, 0x42, 0x7E,
        0x1B, 0x28, 0x4A, 0x7E, 0x1B, 0x24, 0x42, 0x24, 0x22,
        0x1B, 0x28, 0x42
    };

    encoding = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    for (length = 5; length <= sizeof(buffer); length++) {
        status = lxb_encoding_encode_init(&encode, encoding, buffer, length);
        test_eq(status, LXB_STATUS_OK);

        cps = source;
        end = source + (sizeof(source) / sizeof(lxb_codepoint_t));
        used = 0;

        do {
            status = encoding->encode(&encode, &cps, end);

            memcpy(&out[used], buffer, lxb_encoding_encode_buf_used(&encode));
            used += lxb_encoding_encode_buf_used(&encode);

            lxb_encoding_encode_buf_used_set(&encode, 0);
        }
        while (status == LXB_STATUS_SMALL_BUFFER);

        test_eq(status, LXB_STATUS_OK);

        status = lxb_encoding_encode_finish(&encode);
        test_eq(status, LXB_STATUS_OK);

        memcpy(&out[used], buffer, lxb_encoding_encode_buf_used(&encode));
        used += lxb_encoding_encode_buf_used(&encode);

        test_eq_str_n(out, used, result, sizeof(result));
    }
}
TEST_END

TEST_BEGIN(size_fix)
{
    lxb_status_t status;
    lxb_encoding_encode_t encode;
    lxb_char_t buffer[4096];

    const lxb_codepoint_t *cps, *end;
    const lxb_encoding_data_t *enc_data;
    const lxb_char_t replacement[] = "?";

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);
    test_ne(enc_data, NULL);

    status = lxb_encoding_encode_init(&encode, enc_data, buffer, sizeof(buffer));
    test_eq(status, LXB_STATUS_OK);

    status = lxb_encoding_encode_replace_set(&encode, replacement, 1);
    test_eq(status, LXB_STATUS_OK);

    lxb_codepoint_t codepoints[] = {
        0x3042,  /* Hiragana A (あ) - in JIS0208 -> triggers state switch */
        0x0400,  /* Cyrillic - NOT in JIS0208 -> stale size decrement */
        0x0400,  /* Again -> further corruption */
        0x0400,  /* Again -> underflow to SIZE_MAX-2 */
    };

    cps = codepoints;
    end = codepoints + (sizeof(codepoints) / sizeof(codepoints[0]));

    printf("Before encode: buffer_used = %zu\n", encode.buffer_used);

    status = lxb_encoding_encode_iso_2022_jp(&encode, &cps, end);

    printf("After encode:  buffer_used = %zu, status = %d\n",
           encode.buffer_used, (int) status);

    if (encode.buffer_used > sizeof(buffer)) {
        TEST_FAILURE("encode.buffer_used > sizeof(buffer)");
    }
}
TEST_END

int
main(int argc, const char * argv[])
{
    if (argc != 2) {
        printf("Usage:\n\tiso_2022_jp <filepath>\n");
        return EXIT_FAILURE;
    }

    lxb_filepath_test = argv[1];

    TEST_INIT();

    TEST_ADD(decode);
    TEST_ADD(decode_prepend);
    TEST_ADD(decode_buffer_check_escape_dollar);
    TEST_ADD(decode_buffer_check_escape_paren);
    TEST_ADD(decode_buffer_check_escape_start);
    TEST_ADD(decode_buffer_check_trail);
    TEST_ADD(decode_finish_after_escape);
    TEST_ADD(decode_finish_escape);
    TEST_ADD(decode_buffer_check_finish_escape);
    TEST_ADD(decode_map);
    TEST_ADD(encode_map);
    TEST_ADD(encode_buffer_check);
    TEST_ADD(encode_sequence);
    TEST_ADD(encode_sequence_small_buffer);
    TEST_ADD(size_fix);

    TEST_RUN("lexbor/encoding/iso_2022_jp");
    TEST_RELEASE();
}
