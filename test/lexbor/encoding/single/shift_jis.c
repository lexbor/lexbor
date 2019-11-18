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

    enc_data = lxb_encoding_data(LXB_ENCODING_SHIFT_JIS);
    test_ne(enc_data, NULL);

    to_update_buffer("\x58");
    test_buffer(test_decode_chunks, 1, 88);
    test_buffer(test_decode_full, 1, 88);

    /* Range 0xA1 to 0xDF, inclusive */
    to_update_buffer("\xA0");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\xE1");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);

    to_update_buffer("\xA1");
    test_buffer(test_decode_chunks, 1, 0xFF61);
    test_buffer(test_decode_full, 1, 0xFF61);

    to_update_buffer("\xDF");
    test_buffer(test_decode_chunks, 1, 0xFF9F);
    test_buffer(test_decode_full, 1, 0xFF9F);

    /* Range 0x81 to 0x9F, inclusive, or 0xE0 to 0xFC to lead */
    to_update_buffer("\x81");
    test_buffer(test_decode_chunks, 1, LXB_ENCODING_DECODE_CONTINUE);
    test_buffer(test_decode_full, 1, LXB_ENCODING_DECODE_CONTINUE);

    to_update_buffer("\x81\x40");
    test_buffer(test_decode_chunks, 1, 0x3000);
    test_buffer(test_decode_full, 1, 0x3000);

    to_update_buffer("\x81\x7E");
    test_buffer(test_decode_chunks, 1, 0xD7);
    test_buffer(test_decode_full, 1, 0xD7);

    to_update_buffer("\x81\x80");
    test_buffer(test_decode_chunks, 1, 0xF7);
    test_buffer(test_decode_full, 1, 0xF7);

    to_update_buffer("\x81\xFC");
    test_buffer(test_decode_chunks, 1, 0x25EF);
    test_buffer(test_decode_full, 1, 0x25EF);

    to_update_buffer("\x9F\xFC");
    test_buffer(test_decode_chunks, 1, 0x6ECC);
    test_buffer(test_decode_full, 1, 0x6ECC);

    to_update_buffer("\xFC\xFC");
    test_buffer(test_decode_chunks, 1, rp_cp);
    test_buffer(test_decode_full, 1, rp_cp);
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

    enc_data = lxb_encoding_data(LXB_ENCODING_SHIFT_JIS);
    test_ne(enc_data, NULL);

    to_update_buffer("\xFF\x9F\xFC");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x6ECC);
    test_buffer(test_decode_full, 2, rp_cp, 0x6ECC);

    to_update_buffer("\xFC\xFC\x41");
    test_buffer(test_decode_chunks, 2, rp_cp, 0x41);
    test_buffer(test_decode_full, 2, rp_cp, 0x41);

    to_update_buffer("\xFC\x7E\x41");
    test_buffer(test_decode_chunks, 3, rp_cp, 0x7E, 0x41);
    test_buffer(test_decode_full, 3, rp_cp, 0x7E, 0x41);
}
TEST_END

TEST_BEGIN(decode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_SHIFT_JIS);

    status = test_encoding_process_file(lxb_filepath_test, test_decode_process_file,
                                        (void *) enc_data, &line);
    if (status != LXB_STATUS_OK) {
        failed_and_exit(line);
    }
}
TEST_END

TEST_BEGIN(encode)
{
    int8_t size;
    lxb_char_t ch1;
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_SHIFT_JIS);

    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0x00A5);
    test_eq(size, 1);
    test_eq(ch1, 0x5C);

    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0x203E);
    test_eq(size, 1);
    test_eq(ch1, 0x7E);

}
TEST_END

TEST_BEGIN(encode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_SHIFT_JIS);

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
    lxb_char_t ch1, ch2[2];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_SHIFT_JIS);

    /* 2 */
    ref = &ch1;
    size = enc_data->encode_single(&ctx, &ref, ref + 1, 0xFA1F);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode_single(&ctx, &ref, ref + 2, 0xFA1F);
    test_eq(size, 2);
}
TEST_END

int
main(int argc, const char * argv[])
{
    if (argc != 2) {
        printf("Usage:\n\tshift_jis <filepath>\n");
        return EXIT_FAILURE;
    }

    lxb_filepath_test = argv[1];

    TEST_INIT();

    TEST_ADD(decode);
    TEST_ADD(decode_prepend);
    TEST_ADD(decode_map);
    TEST_ADD(encode);
    TEST_ADD(encode_map);
    TEST_ADD(encode_buffer_check);

    TEST_RUN("lexbor/encoding/shift_jis");
    TEST_RELEASE();
}
