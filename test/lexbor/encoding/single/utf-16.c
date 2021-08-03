/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include "encoding.h"


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

    TEST_RUN("lexbor/encoding/utf_16");
    TEST_RELEASE();
}
