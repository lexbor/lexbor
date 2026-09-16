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
    test_buffer(test_decode_chunks, 1, 0x10000);
    test_buffer(test_decode_full, 1, 0x10000);

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

TEST_BEGIN(encode)
{
    int8_t size;
    lxb_char_t ch4[4];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16BE);

    /*
     * Make GCC happy.
     * Why here? warning: 'ch4' may be used uninitialized.
     * Why not in the other functions of this file? Where it's the same.
     * The "encode_single" callback does not read the buffer, it only fills it.
     */
    memset(ch4, 0x00, sizeof(ch4));

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 2, 0x9FFF);
    test_eq(size, 2);
    test_eq_u_str_n(ch4, 2, "\x9F\xFF", 2);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x10000);
    test_eq(size, 4);
    test_eq_u_str_n(ch4, 4, "\xD8\x00\xDC\x00", 4);
}
TEST_END

TEST_BEGIN(encode_buffer_check)
{
    int8_t size;
    lxb_char_t ch1, ch2[2], ch3[3], ch4[4];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_16BE);

    /* 4 */
    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0x10000);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode_single(&ctx, &ref, ref + 2, 0x10000);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch3;
    size = enc_data->encode_single(&ctx, &ref, ref + 3, 0x10000);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x10000);
    test_eq(size, 4);
    test_eq_u_str_n(ch4, 4, "\xD8\x00\xDC\x00", 4);
}
TEST_END

TEST_BEGIN(encode_surrogate_and_range)
{
    int8_t size;
    lxb_char_t out[8];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    static const lxb_encoding_t encodings[] = {
        LXB_ENCODING_UTF_16BE, LXB_ENCODING_UTF_16LE
    };

    static const lxb_codepoint_t cps[] = {
        0xD800, 0xDBFF, 0xDC00, 0xDFFF,
        0x110000, 0x210000, UINT32_MAX
    };

    for (size_t i = 0; i < sizeof(encodings) / sizeof(encodings[0]); i++) {
        enc_data = lxb_encoding_data(encodings[i]);
        test_ne(enc_data, NULL);

        for (size_t j = 0; j < sizeof(cps) / sizeof(cps[0]); j++) {
            for (size_t capacity = 0; capacity <= 4; capacity++) {
                memset(out, 0xA5, sizeof(out));
                ref = out;
                size = enc_data->encode_single(&ctx, &ref, out + capacity,
                                                cps[j]);
                test_eq(size, LXB_ENCODING_ENCODE_ERROR);
                test_eq(ref, out);

                for (size_t k = 0; k < sizeof(out); k++) {
                    test_eq(out[k], 0xA5);
                }
            }
        }
    }
}
TEST_END

TEST_BEGIN(encode_boundaries)
{
    int8_t size;
    lxb_char_t out[8];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
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
                memset(out, 0xA5, sizeof(out));
                ref = out;
                size = enc_data->encode_single(&ctx, &ref, out + capacity,
                                                entries[j].cp);

                if (capacity < entries[j].length) {
                    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);
                    test_eq(ref, out);

                    for (size_t k = 0; k < sizeof(out); k++) {
                        test_eq(out[k], 0xA5);
                    }

                    size = enc_data->encode_single(&ctx, &ref,
                                 out + entries[j].length, entries[j].cp);
                }

                test_eq_size(size, entries[j].length);
                test_eq(ref, out + entries[j].length);
                test_eq_u_str_n(out, entries[j].length,
                               entries[j].bytes[i], entries[j].length);

                for (size_t k = entries[j].length; k < sizeof(out); k++) {
                    test_eq(out[k], 0xA5);
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
    TEST_ADD(encode);
    TEST_ADD(encode_buffer_check);
    TEST_ADD(encode_surrogate_and_range);
    TEST_ADD(encode_boundaries);

    TEST_RUN("lexbor/encoding/utf_16");
    TEST_RELEASE();
}
