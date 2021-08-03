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
    test_buffer(test_decode_chunks, 2, rp_cp, LXB_TEST_ENCODING_CONTINUE_CODEPOINT);
    test_buffer(test_decode_full, 2, rp_cp, LXB_TEST_ENCODING_CONTINUE_CODEPOINT);

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
    lxb_status_t status;
    lxb_char_t ch4[4];

    lxb_codepoint_t cp;
    const lxb_codepoint_t *cps;
    lxb_encoding_encode_t enctx;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_8);

    lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));

    /* 1 */
    cp = 0x7F;

    cps = &cp;
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 1);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\x7F", 1);

    /* 2 */
    cp = 0x80;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 2);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xC2\x80", 2);

    /* 2 */
    cp = 0x07FF;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 2);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xDF\xBF", 2);

    /* 3 */
    cp = 0x0800;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 3);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xE0\xA0\x80", 3);

    /* 3 */
    cp = 0xFFFF;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 3);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xEF\xBF\xBF", 3);

    /* 4 */
    cp = 0x10000;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 4);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xF0\x90\x80\x80", 4);

    /* 4 */
    cp = 0x10FFFF;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 4);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xF4\x8F\xBF\xBF", 4);

    /* 4 */
    cp = 0x110000;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, ch4, sizeof(ch4));
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_ERROR);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);
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

    enc_data = lxb_encoding_data(LXB_ENCODING_UTF_8);

    lxb_encoding_encode_init(&enctx, enc_data, &ch1, sizeof(ch1));

    /* 2 */
    cp = 0x07FF;

    cps = &cp;
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch2, 2);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 2);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xDF\xBF", 2);

    /* 3 */
    cp = 0xFFFF;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, &ch1, sizeof(ch1));
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch2, 2);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_SMALL_BUFFER);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 0);

    cps = &cp; lxb_encoding_encode_buf_set(&enctx, ch3, 3);
    status = enc_data->encode(&enctx, &cps, cps + 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_encoding_encode_buf_used(&enctx), 3);
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xEF\xBF\xBF", 3);

    /* 4 */
    cp = 0x10FFFF;

    cps = &cp; lxb_encoding_encode_init(&enctx, enc_data, &ch1, sizeof(ch1));
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
    test_eq_u_str_n(enctx.buffer_out, enctx.buffer_used, "\xF4\x8F\xBF\xBF", 4);
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
