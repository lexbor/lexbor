/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include "encoding.h"


TEST_BEGIN(decode)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_8);
    test_ne(enc_data, NULL);

    to_update_buffer("\x32");
    test_buffer(test_decode_chunks, 1, 0x32);
    test_buffer(test_decode_full, 1, 0x32);

    to_update_buffer("\xC2\x32");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x32);
    test_buffer(test_decode_full, 2, rp_cp, 0x32);

    to_update_buffer("\xC2\x80");
    test_buffer(test_decode_chunks, 1, 0x80);
    test_buffer(test_decode_full, 1, 0x80);

    to_update_buffer("\xDF\x80");
    test_buffer(test_decode_chunks, 1, 0x07C0);
    test_buffer(test_decode_full, 1, 0x07C0);

    to_update_buffer("\xE0\xA0\x80");
    test_buffer(test_decode_chunks, 1, 0x0800);
    test_buffer(test_decode_full, 1, 0x0800);

    to_update_buffer("\xE0\xA0\x80");
    test_buffer(test_decode_chunks, 1, 0x0800);
    test_buffer(test_decode_full, 1, 0x0800);

    to_update_buffer("\xE0\x7F\xC2\x80");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x7F, 0x80);
    test_buffer(test_decode_full, 3, rp_cp, 0x7F, 0x80);

    to_update_buffer("\xE0\xBF\x80");
    test_buffer(test_decode_chunks, 1, 0x0FC0);
    test_buffer(test_decode_full, 1, 0x0FC0);

    to_update_buffer("\xE0\xC0\xC2\x80");
    test_buffer(test_decode_chunks, 3, rp_cp, rp_cp, 0x80);
    test_buffer(test_decode_full, 3, rp_cp, rp_cp, 0x80);

    to_update_buffer("\xE0\xED\x80");
    test_buffer(test_decode_chunks, 2, rp_cp, LXB_ENCODING_DECODE_CONTINUE);
    test_buffer(test_decode_full, 2, rp_cp, LXB_ENCODING_DECODE_CONTINUE);

    to_update_buffer("\xED\x9F\x80");
    test_buffer(test_decode_chunks, 1, 0xD7C0);
    test_buffer(test_decode_full, 1, 0xD7C0);

    to_update_buffer("\xED\xA0\x80");
    test_buffer(test_decode_chunks, 3, rp_cp, rp_cp, rp_cp);
    test_buffer(test_decode_full, 3, rp_cp, rp_cp, rp_cp);

    to_update_buffer("\xF0\x90\x80\x80");
    test_buffer(test_decode_chunks, 1, 0x10000);
    test_buffer(test_decode_full, 1, 0x10000);

    to_update_buffer("\xF0\x8F\x80\x80");
    test_buffer(test_decode_chunks, 4, rp_cp, rp_cp, rp_cp, rp_cp);
    test_buffer(test_decode_full, 4, rp_cp, rp_cp, rp_cp, rp_cp);

    to_update_buffer("\xF0\x8F\x80\xC2\x80");
    test_buffer(test_decode_chunks, 4, rp_cp, rp_cp, rp_cp, 0x80);
    test_buffer(test_decode_full, 4, rp_cp, rp_cp, rp_cp, 0x80);

    to_update_buffer("\xF4\x8F\x80\x80");
    test_buffer(test_decode_chunks, 1, 0x10F000);
    test_buffer(test_decode_full, 1, 0x10F000);

    to_update_buffer("\xF4\x90\x80\x80");
    test_buffer(test_decode_chunks, 4, rp_cp, rp_cp, rp_cp, rp_cp);
    test_buffer(test_decode_full, 4, rp_cp, rp_cp, rp_cp, rp_cp);

    to_update_buffer("\xF4\x90\x80\xC2\x80");
    test_buffer(test_decode_chunks, 4, rp_cp, rp_cp, rp_cp, 0x80);
    test_buffer(test_decode_full, 4, rp_cp, rp_cp, rp_cp, 0x80);

    to_update_buffer("\xF4\x8F\x80\x32");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x32);
    test_buffer(test_decode_full, 2, rp_cp, 0x32);

    to_update_buffer("\xD0\xB8\xD0\xBD");
    test_buffer(test_decode_chunks, 2, 0x0438, 0x043D);
    test_buffer(test_decode_full, 2, 0x0438, 0x043D);
}
TEST_END

TEST_BEGIN(encode)
{
    int8_t size;
    lxb_char_t ch4[4];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_8);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x7F);
    test_eq(size, 1);
    test_eq_u_str_n(ch4, 1, "\x7F", 1);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x80);
    test_eq(size, 2);
    test_eq_u_str_n(ch4, 2, "\xC2\x80", 2);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x07FF);
    test_eq(size, 2);
    test_eq_u_str_n(ch4, 2, "\xDF\xBF", 2);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x0800);
    test_eq(size, 3);
    test_eq_u_str_n(ch4, 3, "\xE0\xA0\x80", 3);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0xFFFF);
    test_eq(size, 3);
    test_eq_u_str_n(ch4, 3, "\xEF\xBF\xBF", 3);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x10000);
    test_eq(size, 4);
    test_eq_u_str_n(ch4, 4, "\xF0\x90\x80\x80", 4);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x10FFFF);
    test_eq(size, 4);
    test_eq_u_str_n(ch4, 4, "\xF4\x8F\xBF\xBF", 4);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x110000);
    test_eq(size, LXB_ENCODING_ENCODE_ERROR);
}
TEST_END

TEST_BEGIN(encode_buffer_check)
{
    int8_t size;
    lxb_char_t ch1, ch2[2], ch3[3], ch4[4];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_8);

    /* 2 */
    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0x07FF);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode_single(&ctx, &ref, ref + 2, 0x07FF);
    test_eq(size, 2);
    test_eq_u_str_n(ch2, 2, "\xDF\xBF", 2);

    /* 3 */
    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0xFFFF);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode_single(&ctx, &ref, ref + 2, 0xFFFF);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch3;
    size = enc_data->encode_single(&ctx, &ref, ref + 3, 0xFFFF);
    test_eq(size, 3);
    test_eq_u_str_n(ch3, 3, "\xEF\xBF\xBF", 3);

    /* 4 */
    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0x10FFFF);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode_single(&ctx, &ref, ref + 2, 0x10FFFF);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch3;
    size = enc_data->encode_single(&ctx, &ref, ref + 3, 0x10FFFF);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x10FFFF);
    test_eq(size, 4);
    test_eq_u_str_n(ch4, 4, "\xF4\x8F\xBF\xBF", 4);
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

    TEST_ADD(decode);
    TEST_ADD(encode);
    TEST_ADD(encode_buffer_check);

    TEST_RUN("lexbor/encoding/utf_8");
    TEST_RELEASE();
}
