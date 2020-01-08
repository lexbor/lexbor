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
    lxb_codepoint_t rp_cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    lxb_codepoint_t cps_buffer[1024];

    enc_data = lxb_encoding_data(LXB_ENCODING_GB18030);
    test_ne(enc_data, NULL);

    /* UTF-8: \x58; Unicode: \x58\x00; Code point: 88 */
    to_update_buffer("\x58");
    test_buffer(test_decode_chunks, 1, 88);
    test_buffer(test_decode_full, 1, 88);

    /* UTF-8: \xE2\x82\xAC; Unicode: \x20\xAC; Code point: 8364 */
    to_update_buffer("\x80");
    test_buffer(test_decode_chunks, 1, 8364);
    test_buffer(test_decode_full, 1, 8364);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\xFF");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    /* UTF-8: \xE4\xBA\x8A; Unicode: \x4E\x8A; Code point: 20106 */
    to_update_buffer("\x81\x7E");
    test_buffer(test_decode_chunks, 1, 20106);
    test_buffer(test_decode_full, 1, 20106);

    /* UTF-8: \xE4\xBA\xB8; Unicode: \x4E\xB8; Code point: 20152 */
    to_update_buffer("\x81\x8F");
    test_buffer(test_decode_chunks, 1, 20152);
    test_buffer(test_decode_full, 1, 20152);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x81\x29");
    test_buffer(test_decode_chunks, 2, rp_cp, 41);
    test_buffer(test_decode_full, 2, rp_cp, 41);

    /* UTF-8: \xE4\xB8\x82; Unicode: \x4E\x02; Code point: 19970 */
    to_update_buffer("\x81\x40");
    test_buffer(test_decode_chunks, 1, 19970);
    test_buffer(test_decode_full, 1, 19970);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x81\x30\x80");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x30, 0x20AC);
    test_buffer(test_decode_full, 3, rp_cp, 0x30, 0x20AC);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x81\x30\x81\x29");
    test_buffer(test_decode_chunks, 4, rp_cp, 0x30, rp_cp, 0x29);
    test_buffer(test_decode_full, 4, rp_cp, 0x30, rp_cp, 0x29);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x81\xFF");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    /* UTF-8: \xE2\x8F\xAD; Unicode: \x23\xED; Code point: 9197 */
    to_update_buffer("\x81\x37\x81\x31");
    test_buffer(test_decode_chunks, 1, 9197);
    test_buffer(test_decode_full, 1, 9197);

    /* UTF-8: \xC2\x80; Unicode: \x80; Code point: 128 */
    to_update_buffer("\x81\x30\x81\x30");
    test_buffer(test_decode_chunks, 1, 128);
    test_buffer(test_decode_full, 1, 128);

    /* UTF-8: \xC2\x80; Unicode: \x80; Code point: 128 */
    to_update_buffer("\x81\x30\x81\x30");
    test_buffer(test_decode_chunks, 1, 128);
    test_buffer(test_decode_full, 1, 128);

    /* UTF-8: \xE2\xBA\x9B; Unicode: \x2E\x9B; Code point: 11931 */
    to_update_buffer("\x81\x39\x81\x39");
    test_buffer(test_decode_chunks, 1, 11931);
    test_buffer(test_decode_full, 1, 11931);

    /* UTF-8: \xE2\xBA\x9B; Unicode: \x2E\x9B; Code point: 11931 */
    to_update_buffer("\x81\x39\x81\x39");
    test_buffer(test_decode_chunks, 1, 11931);
    test_buffer(test_decode_full, 1, 11931);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\xFE\x30\x81\x30");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\xFE\x30\xFE\x30");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\xFE\x39\xFE\x39");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    /* UTF-8: \xD8\x80; Unicode: \x06\x00; Code point: 1536 */
    to_update_buffer("\x81\x30\xFE\x30");
    test_buffer(test_decode_chunks, 1, 1536);
    test_buffer(test_decode_full, 1, 1536);

    /* UTF-8: \xE3\x92\xA2; Unicode: \x34\xA2; Code point: 13474 */
    to_update_buffer("\x81\x39\xFE\x39");
    test_buffer(test_decode_chunks, 1, 13474);
    test_buffer(test_decode_full, 1, 13474);

    to_update_buffer("\x81\x39\xFE\x39\x81\x39\xFE\x39");
    test_buffer(test_decode_chunks, 2, 13474, 13474);
    test_buffer(test_decode_full, 2, 13474, 13474);
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

    enc_data = lxb_encoding_data(LXB_ENCODING_GB18030);
    test_ne(enc_data, NULL);

    /* Fisrt */
    to_update_buffer("\x81\x7F");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x7F);
    test_buffer(test_decode_full, 2, rp_cp, 0x7F);

    /* Second */
    to_update_buffer("\x81\x30\x20");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x30, 0x20);
    test_buffer(test_decode_full, 3, rp_cp, 0x30, 0x20);

    to_update_buffer("\x81\x30\x80");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x30, 0x20AC);
    test_buffer(test_decode_full, 3, rp_cp, 0x30, 0x20AC);

    to_update_buffer("\x81\x30\xFF");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x30, rp_cp);
    test_buffer(test_decode_full, 3, rp_cp, 0x30, rp_cp);

    to_update_buffer("\x81\x30\xFF\x81\x37\x81\x31");
    test_buffer(test_decode_chunks, 4, rp_cp, 0x30, rp_cp, 0x23ED);
    test_buffer(test_decode_full, 4, rp_cp, 0x30, rp_cp, 0x23ED);

    to_update_buffer("\x81\xFF\x81\x81\x37\x81\x31");
    test_buffer(test_decode_chunks, 4, rp_cp, 0x4E96, 0x37, LXB_ENCODING_DECODE_CONTINUE);
    test_buffer(test_decode_full, 4, rp_cp, 0x4E96, 0x37, LXB_ENCODING_DECODE_CONTINUE);

    to_update_buffer("\x81\xFF\x81\x81\x37\x81\x31\x81\x31");
    test_buffer(test_decode_chunks, 4, rp_cp, 0x4E96, 0x37, 0x060B);
    test_buffer(test_decode_full, 4, rp_cp, 0x4E96, 0x37, 0x060B);

    /* Third */
    to_update_buffer("\x81\x30\x81\x81\x37\x81\x31");
    test_buffer(test_decode_full, 5, rp_cp, 0x30, 0x4E96, 0x37, LXB_ENCODING_DECODE_CONTINUE);
    test_buffer(test_decode_chunks, 5, rp_cp, 0x30, 0x4E96, 0x37, LXB_ENCODING_DECODE_CONTINUE);

    to_update_buffer("\x81\x30\x81\x40");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x30, 0x4E02);
    test_buffer(test_decode_full, 3, rp_cp, 0x30, 0x4E02);
}
TEST_END

TEST_BEGIN(decode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_GB18030);

    status = test_encoding_process_file(lxb_filepath_test, test_decode_process_file,
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

    enc_data = lxb_encoding_data(LXB_ENCODING_GB18030);

    status = test_encoding_process_file(lxb_filepath_test, test_encode_process_file,
                                        (void *) enc_data, &line);
    if (status != LXB_STATUS_OK) {
        failed_and_exit(line);
    }
}
TEST_END

TEST_BEGIN(encode_buffer_check)
{
    int8_t size;
    lxb_char_t ch1, ch2[2], ch3[3], ch4[4];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_GB18030);

    /* 4 */
    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0x022E);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode_single(&ctx, &ref, ref + 2, 0x022E);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch3;
    size = enc_data->encode_single(&ctx, &ref, ref + 3, 0x022E);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch4;
    size = enc_data->encode_single(&ctx, &ref, ref + 4, 0x022E);
    test_eq(size, 4);

    /* 2 */
    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0x5ABE);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode_single(&ctx, &ref, ref + 2, 0x5ABE);
    test_eq(size, 2);
}
TEST_END

int
main(int argc, const char * argv[])
{
    if (argc != 2) {
        printf("Usage:\n\tgb18030 <filepath>\n");
        return EXIT_FAILURE;
    }

    lxb_filepath_test = argv[1];

    TEST_INIT();

    TEST_ADD(decode);
    TEST_ADD(decode_prepend);
    TEST_ADD(decode_map);
    TEST_ADD(encode_map);
    TEST_ADD(encode_buffer_check);

    TEST_RUN("lexbor/encoding/gb18030");
    TEST_RELEASE();
}
