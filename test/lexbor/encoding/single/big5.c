/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include "encoding.h"


static const char *lxb_filepath_test;


TEST_BEGIN(decode)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    size_t size;
    lxb_codepoint_t cps_buffer[1024];

    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    enc_data = lxb_encoding_data(LXB_ENCODING_BIG5);
    test_ne(enc_data, NULL);

    /* UTF-8: \x58; Unicode: \x58\x00; Code point: 88 */
    to_update_buffer("\x58");
    test_buffer(test_decode_chunks, 1, 88);
    test_buffer(test_decode_full, 1, 88);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x80");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\xFF");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x81\x7F");
    test_buffer(test_decode_chunks, 2, rp_cp, 127);
    test_buffer(test_decode_full, 2, rp_cp, 127);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x81\x40");
    test_buffer(test_decode_chunks, 2, rp_cp, 64);
    test_buffer(test_decode_full, 2, rp_cp, 64);

    /* UTF-8: \xE7\xA7\x94; Unicode: \x79\xD4; Code point: 31188 */
    to_update_buffer("\xFE\xFE");
    test_buffer(test_decode_chunks, 1, 31188);
    test_buffer(test_decode_full, 1, 31188);

    /* UTF-8: \xF0\xA4\xAA\x8C; Unicode: \xD8\x52\xDE\x8C; Code point: 150156 */
    to_update_buffer("\xFE\x7E");
    test_buffer(test_decode_chunks, 1, 150156);
    test_buffer(test_decode_full, 1, 150156);

    /* UTF-8: \xC3\x8A\xCC\x84; Unicode: \x00\xCA\x03\x04; Code point: 202 and 772 */
    to_update_buffer("\x88\x62");
    test_buffer(test_decode_full, 2, 202, 772);
    test_buffer(test_decode_chunks, 2, 202, 772);

    /* UTF-8: \xC3\x8A\xCC\x8C; Unicode: \x00\xCA\x03\x0C; Code point: 202 and 780 */
    to_update_buffer("\x88\x64");
    test_buffer(test_decode_full, 2, 202, 780);
    test_buffer(test_decode_chunks, 2, 202, 780);

    /* UTF-8: \xC3\xAA\xCC\x84; Unicode: \x00\xEA\x03\x04; Code point: 234 and 772 */
    to_update_buffer("\x88\xA3");
    test_buffer(test_decode_full, 2, 234, 772);
    test_buffer(test_decode_chunks, 2, 234, 772);

    /* UTF-8: \xC3\xAA\xCC\x8C; Unicode: \x00\xEA\x03\x0C; Code point: 234 and 780 */
    to_update_buffer("\x88\xA5");
    test_buffer(test_decode_full, 2, 234, 780);
    test_buffer(test_decode_chunks, 2, 234, 780);

    to_update_buffer("\xFE\x7E\xFE\x7E");
    test_buffer(test_decode_full, 2, 150156, 150156);
    test_buffer(test_decode_chunks, 2, 150156, 150156);
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

    enc_data = lxb_encoding_data(LXB_ENCODING_BIG5);
    test_ne(enc_data, NULL);

    to_update_buffer("\x8F\x7F");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x7F);
    test_buffer(test_decode_full, 2, rp_cp, 0x7F);

    to_update_buffer("\x81\x32");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x32);
    test_buffer(test_decode_full, 2, rp_cp, 0x32);

    to_update_buffer("\x81\xFF");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\x81\xFF\xFE\x7E");
    test_buffer(test_decode_chunks, 2, rp_cp, 150156);
    test_buffer(test_decode_full, 2, rp_cp, 150156);

    to_update_buffer("\x81\x40\x32");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x40, 0x32);
    test_buffer(test_decode_full, 3, rp_cp, 0x40, 0x32);
}
TEST_END

TEST_BEGIN(decode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_BIG5);

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

    enc_data = lxb_encoding_data(LXB_ENCODING_BIG5);

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
    int8_t size;
    lxb_char_t ch1, ch2[2];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_BIG5);

    /* 2 */
    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0x9AD0);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode_single(&ctx, &ref, ref + 2, 0x9AD0);
    test_eq(size, 2);

    /* 1 */
    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0x9AD0);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);
}
TEST_END

int
main(int argc, const char * argv[])
{
    if (argc != 2) {
        printf("Usage:\n\tbig5 <filepath>\n");
        return EXIT_FAILURE;
    }

    lxb_filepath_test = argv[1];

    TEST_INIT();

    TEST_ADD(decode);
    TEST_ADD(decode_prepend);
    TEST_ADD(decode_map);
    TEST_ADD(encode_map);
    TEST_ADD(encode_buffer_check);

    TEST_RUN("lexbor/encoding/big5");
    TEST_RELEASE();
}
