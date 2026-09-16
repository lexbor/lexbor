/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include "encoding.h"


typedef struct {
    lxb_codepoint_t cp;
    size_t          length;
    lxb_char_t      bytes[2][4];
}
utf_16_encode_entry_t;


TEST_BEGIN(decode_be)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16BE);
    test_ne(enc_data, NULL);

    to_update_buffer("\x9F\xFF");
    test_buffer(test_decode_chunks, 1, 0x9fff);
    test_buffer(test_decode_full, 1, 0x9fff);

    to_update_buffer_size("\x00\x80", 2);
    test_buffer(test_decode_chunks, 1, 0x0080);
    test_buffer(test_decode_full, 1, 0x0080);

    to_update_buffer_size("\x00\x32", 2);
    test_buffer(test_decode_chunks, 1, 0x0032);
    test_buffer(test_decode_full, 1, 0x0032);

    to_update_buffer_size("\xD8\x00\xDC\x00", 4);
    test_buffer(test_decode_full, 1, 0x10000);
    test_buffer(test_decode_chunks, 1, 0x10000);

    to_update_buffer("\xDB\xFF\xDF\xFF");
    test_buffer(test_decode_chunks, 1, 0x10FFFF);
    test_buffer(test_decode_full, 1, 0x10FFFF);

    to_update_buffer_size("\xDC\x00\xDC\x00", 4);
    test_buffer(test_decode_chunks, 2, rp_cp, rp_cp);
    test_buffer(test_decode_full, 2, rp_cp, rp_cp);

    to_update_buffer_size("\xDC\x00\xD8\x00\xDC\x00", 6);
    test_buffer(test_decode_chunks, 2, rp_cp, 0x10000);
    test_buffer(test_decode_full, 2, rp_cp, 0x10000);

    to_update_buffer("\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF");
    test_buffer(test_decode_chunks, 2, 0x10FFFF, 0x10FFFF);
    test_buffer(test_decode_full, 2, 0x10FFFF, 0x10FFFF);
}
TEST_END

/* Broken encoding. Prepend to stream test. */
TEST_BEGIN(decode_be_prepend)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16BE);
    test_ne(enc_data, NULL);

    to_update_buffer_size("\xD8\x00\xD8\x00\xDC\x00", 6);
    test_buffer(test_decode_full, 2, rp_cp, 0x10000);
    test_buffer(test_decode_chunks, 2, rp_cp, 0x10000);
}
TEST_END

TEST_BEGIN(decode_le)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16LE);
    test_ne(enc_data, NULL);

    to_update_buffer("\xFF\x9F");
    test_buffer(test_decode_chunks, 1, 0x9fff);
    test_buffer(test_decode_full, 1, 0x9fff);

    to_update_buffer_size("\x80\x00", 2);
    test_buffer(test_decode_chunks, 1, 0x80);
    test_buffer(test_decode_full, 1, 0x80);

    to_update_buffer_size("\x32\x00", 2);
    test_buffer(test_decode_chunks, 1, 0x32);
    test_buffer(test_decode_full, 1, 0x32);

    to_update_buffer_size("\x00\xD8\x00\xDC", 4);
    test_buffer(test_decode_chunks, 1, 0x10000);
    test_buffer(test_decode_full, 1, 0x10000);

    to_update_buffer("\xFF\xDB\xFF\xDF");
    test_buffer(test_decode_chunks, 1, 0x10FFFF);
    test_buffer(test_decode_full, 1, 0x10FFFF);

    to_update_buffer_size("\x00\xDC\x00\xDC", 4);
    test_buffer(test_decode_chunks, 2, rp_cp, rp_cp);
    test_buffer(test_decode_full, 2, rp_cp, rp_cp);

    to_update_buffer_size("\x00\xDC\x00\xD8\x00\xDC", 6);
    test_buffer(test_decode_chunks, 2, rp_cp, 0x10000);
    test_buffer(test_decode_full, 2, rp_cp, 0x10000);
}
TEST_END

TEST_BEGIN(decode_le_prepend)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16LE);
    test_ne(enc_data, NULL);

    to_update_buffer_size("\x00\xD8\x00\xD8\x00\xDC", 6);
    test_buffer(test_decode_full, 2, rp_cp, 0x10000);
    test_buffer(test_decode_chunks, 2, rp_cp, 0x10000);
}
TEST_END

TEST_BEGIN(decode_be_buffer_check_prepend)
{
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;
    lxb_encoding_decode_t dectx;
    const lxb_char_t *data;
    lxb_codepoint_t resume[1];
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;

    static const lxb_char_t invalid_prepend[] = {0xD8, 0x00, 0x00, 0x41};

    struct {
        lxb_codepoint_t out[1];
        lxb_codepoint_t canary;
    } storage = {{0}, 0xA5A5A5A5};

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16BE);
    test_ne(enc_data, NULL);

    data = invalid_prepend;

    status = lxb_encoding_decode_init(&dectx, enc_data, storage.out, 1);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_encoding_decode_replace_set(&dectx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    test_eq(status, LXB_STATUS_OK);

    status = enc_data->decode(&dectx, &data,
                              invalid_prepend + sizeof(invalid_prepend));
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(storage.out[0], rp_cp);
    test_eq(storage.canary, 0xA5A5A5A5);
    test_eq_size((size_t) (data - invalid_prepend), 3);

    resume[0] = 0;
    lxb_encoding_decode_buf_set(&dectx, resume,
                                sizeof(resume) / sizeof(lxb_codepoint_t));

    status = enc_data->decode(&dectx, &data,
                              invalid_prepend + sizeof(invalid_prepend));
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(resume[0], 0x41);
    test_eq_size((size_t) (data - invalid_prepend), sizeof(invalid_prepend));
}
TEST_END

TEST_BEGIN(decode_le_buffer_check_prepend)
{
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;
    lxb_encoding_decode_t dectx;
    const lxb_char_t *data;
    lxb_codepoint_t resume[1];
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;

    static const lxb_char_t invalid_prepend[] = {0x00, 0xD8, 0x41, 0x00};

    struct {
        lxb_codepoint_t out[1];
        lxb_codepoint_t canary;
    } storage = {{0}, 0xA5A5A5A5};

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16LE);
    test_ne(enc_data, NULL);

    data = invalid_prepend;

    status = lxb_encoding_decode_init(&dectx, enc_data, storage.out, 1);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_encoding_decode_replace_set(&dectx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    test_eq(status, LXB_STATUS_OK);

    status = enc_data->decode(&dectx, &data,
                              invalid_prepend + sizeof(invalid_prepend));
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(storage.out[0], rp_cp);
    test_eq(storage.canary, 0xA5A5A5A5);
    test_eq_size((size_t) (data - invalid_prepend), 3);

    resume[0] = 0;
    lxb_encoding_decode_buf_set(&dectx, resume,
                                sizeof(resume) / sizeof(lxb_codepoint_t));

    status = enc_data->decode(&dectx, &data,
                              invalid_prepend + sizeof(invalid_prepend));
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(lxb_encoding_decode_buf_used(&dectx), 1);
    test_eq(resume[0], 0x41);
    test_eq_size((size_t) (data - invalid_prepend), sizeof(invalid_prepend));
}
TEST_END

TEST_BEGIN(encode)
{
    lxb_status_t status;
    lxb_char_t ch4[4];

    lxb_codepoint_t cp;
    const lxb_codepoint_t *cps;
    lxb_encoding_encode_t enctx;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16BE);

    lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));

    /* 2 */
    cp = 0x9FFF;

    cps = &cp;
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 2);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\x9F\xFF", 2);

    /* 4 */
    cp = 0x10000;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 4);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xD8\x00\xDC\x00", 4);
}
TEST_END

TEST_BEGIN(encode_buffer_check)
{
    lxb_status_t status;
    lxb_char_t ch1, ch2[2], ch3[3], ch4[4];

    lxb_codepoint_t cp;
    const lxb_codepoint_t *cps;
    lxb_encoding_encode_t enctx;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16BE);

    lxb_encoding_encode_init(&enctx, enc_data, &ch1, sizeof(ch1));

    /* 4 */
    cp = 0x10000;

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
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xD8\x00\xDC\x00", 4);
}
TEST_END

TEST_BEGIN(encode_surrogate_and_range)
{
    lxb_status_t status;
    lxb_char_t out[8];
    lxb_codepoint_t cps[] = {0x41, 0, 0x42};
    const lxb_codepoint_t *p;
    lxb_encoding_encode_t ctx;
    const lxb_encoding_data_t *enc_data;

    static const lxb_encoding_t encodings[] = {
        LXB_ENCODING_UTF_16BE, LXB_ENCODING_UTF_16LE
    };

    static const lxb_codepoint_t invalid[] = {
        0xD800, 0xDBFF, 0xDC00, 0xDFFF,
        0x110000, 0x210000, UINT32_MAX
    };

    static const lxb_char_t prefix[][2] = {{0x00, 0x41}, {0x41, 0x00}};

    for (size_t i = 0; i < sizeof(encodings) / sizeof(encodings[0]); i++) {
        enc_data = lxb_encoding_data(encodings[i]);
        test_ne(enc_data, NULL);

        for (size_t j = 0; j < sizeof(invalid) / sizeof(invalid[0]); j++) {
            cps[1] = invalid[j];
            p = cps;
            memset(out, 0xA5, sizeof(out));
            status = lxb_encoding_encode_init(&ctx, enc_data, out, sizeof(out));
            test_eq(status, LXB_STATUS_OK);

            status = enc_data->encode(&ctx, &p, cps + 3);
            test_eq(status, LXB_STATUS_ERROR);
            test_eq(p, cps + 1);
            test_eq_size(ctx.buffer_used, 2);
            test_eq_u_str_n(out, 2, prefix[i], 2);

            for (size_t k = 2; k < sizeof(out); k++) {
                test_eq(out[k], 0xA5);
            }

            /* Invalid input remains an error even without output space. */
            memset(out, 0xA5, sizeof(out));
            lxb_encoding_encode_buf_set(&ctx, out, 0);
            status = enc_data->encode(&ctx, &p, cps + 3);
            test_eq(status, LXB_STATUS_ERROR);
            test_eq(p, cps + 1);
            test_eq_size(ctx.buffer_used, 0);

            for (size_t k = 0; k < sizeof(out); k++) {
                test_eq(out[k], 0xA5);
            }
        }
    }
}
TEST_END

TEST_BEGIN(encode_boundaries)
{
    lxb_status_t status;
    lxb_char_t out[8];
    const lxb_codepoint_t *p;
    lxb_encoding_encode_t ctx;
    const lxb_encoding_data_t *enc_data;

    static const lxb_encoding_t encodings[] = {
        LXB_ENCODING_UTF_16BE, LXB_ENCODING_UTF_16LE
    };

    static const utf_16_encode_entry_t entries[] = {
        {0x0000, 2, {{0x00, 0x00}, {0x00, 0x00}}},
        {0xD7FF, 2, {{0xD7, 0xFF}, {0xFF, 0xD7}}},
        {0xE000, 2, {{0xE0, 0x00}, {0x00, 0xE0}}},
        {0xFFFF, 2, {{0xFF, 0xFF}, {0xFF, 0xFF}}},
        {0x10000, 4, {{0xD8, 0x00, 0xDC, 0x00}, {0x00, 0xD8, 0x00, 0xDC}}},
        {0x10FFFF, 4, {{0xDB, 0xFF, 0xDF, 0xFF}, {0xFF, 0xDB, 0xFF, 0xDF}}}
    };

    for (size_t i = 0; i < sizeof(encodings) / sizeof(encodings[0]); i++) {
        enc_data = lxb_encoding_data(encodings[i]);
        test_ne(enc_data, NULL);

        for (size_t j = 0; j < sizeof(entries) / sizeof(entries[0]); j++) {
            for (size_t capacity = 0; capacity <= entries[j].length; capacity++)
            {
                p = &entries[j].cp;
                memset(out, 0xA5, sizeof(out));
                status = lxb_encoding_encode_init(&ctx, enc_data, out,
                                                  capacity);
                test_eq(status, LXB_STATUS_OK);

                status = enc_data->encode(&ctx, &p, &entries[j].cp + 1);

                if (capacity < entries[j].length) {
                    test_eq(status, LXB_STATUS_SMALL_BUFFER);
                    test_eq(p, &entries[j].cp);
                    test_eq_size(ctx.buffer_used, 0);

                    for (size_t k = 0; k < sizeof(out); k++) {
                        test_eq(out[k], 0xA5);
                    }

                    lxb_encoding_encode_buf_set(&ctx, out, entries[j].length);
                    status = enc_data->encode(&ctx, &p, &entries[j].cp + 1);
                }

                test_eq(status, LXB_STATUS_OK);
                test_eq(p, &entries[j].cp + 1);
                test_eq_size(ctx.buffer_used, entries[j].length);
                test_eq_u_str_n(out, ctx.buffer_used,
                               entries[j].bytes[i], entries[j].length);

                for (size_t k = entries[j].length; k < sizeof(out); k++) {
                    test_eq(out[k], 0xA5);
                }
            }
        }
    }
}
TEST_END

TEST_BEGIN(encode_replacement)
{
    size_t length;
    lxb_status_t status;
    lxb_char_t out[8];
    lxb_codepoint_t cps[] = {0x41, 0, 0x42};
    const lxb_codepoint_t *p;
    lxb_encoding_encode_t ctx;
    const lxb_encoding_data_t *enc_data;

    static const lxb_encoding_t encodings[] = {
        LXB_ENCODING_UTF_16BE, LXB_ENCODING_UTF_16LE
    };

    static const lxb_codepoint_t invalid[] = {
        0xD800, 0xDBFF, 0xDC00, 0xDFFF,
        0x110000, 0x210000, UINT32_MAX
    };

    /* A, U+FFFD, B in the target encoding. */
    static const lxb_char_t expected[][6] = {
        {0x00, 0x41, 0xFF, 0xFD, 0x00, 0x42},
        {0x41, 0x00, 0xFD, 0xFF, 0x42, 0x00}
    };

    for (size_t i = 0; i < sizeof(encodings) / sizeof(encodings[0]); i++) {
        enc_data = lxb_encoding_data(encodings[i]);
        test_ne(enc_data, NULL);

        for (size_t j = 0; j < sizeof(invalid) / sizeof(invalid[0]); j++) {
            cps[1] = invalid[j];

            /* Leave 0..4 bytes for the replacement and the suffix. */
            for (size_t capacity = 2; capacity <= 6; capacity++) {
                p = cps;
                memset(out, 0xA5, sizeof(out));
                status = lxb_encoding_encode_init(&ctx, enc_data, out,
                                                  capacity);
                test_eq(status, LXB_STATUS_OK);
                status = lxb_encoding_encode_replace_set(&ctx, expected[i] + 2,
                                                         2);
                test_eq(status, LXB_STATUS_OK);

                status = enc_data->encode(&ctx, &p, cps + 3);
                test_eq(status, capacity < 6 ? LXB_STATUS_SMALL_BUFFER
                                            : LXB_STATUS_OK);
                length = capacity < 4 ? 2 : capacity < 6 ? 4 : 6;
                test_eq_size(ctx.buffer_used, length);
                test_eq(p, cps + length / 2);
                test_eq_u_str_n(out, length, expected[i], length);

                for (size_t k = length; k < sizeof(out); k++) {
                    test_eq(out[k], 0xA5);
                }

                if (status == LXB_STATUS_SMALL_BUFFER) {
                    memset(out, 0xA5, sizeof(out));
                    lxb_encoding_encode_buf_set(&ctx, out, sizeof(out));
                    status = enc_data->encode(&ctx, &p, cps + 3);
                    test_eq(status, LXB_STATUS_OK);
                    test_eq(p, cps + 3);
                    test_eq_size(ctx.buffer_used, 6 - length);
                    test_eq_u_str_n(out, ctx.buffer_used,
                                   expected[i] + length, 6 - length);

                    for (size_t k = ctx.buffer_used; k < sizeof(out); k++) {
                        test_eq(out[k], 0xA5);
                    }
                }
            }
        }
    }
}
TEST_END

int
main(int argc, const char * argv[])
{
    /* Unused */
    (void) test_encoding_process_file;
    (void) test_encode_process_file;
    (void) test_decode_process_file;

    TEST_INIT();

    TEST_ADD(decode_be);
    TEST_ADD(decode_be_prepend);
    TEST_ADD(decode_le);
    TEST_ADD(decode_le_prepend);
    TEST_ADD(decode_be_buffer_check_prepend);
    TEST_ADD(decode_le_buffer_check_prepend);
    TEST_ADD(encode);
    TEST_ADD(encode_buffer_check);
    TEST_ADD(encode_surrogate_and_range);
    TEST_ADD(encode_boundaries);
    TEST_ADD(encode_replacement);

    TEST_RUN("lexbor/encoding/utf_16");
    TEST_RELEASE();
}
