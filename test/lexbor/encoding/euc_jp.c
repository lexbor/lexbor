/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/encoding/encoding.h>

#include "encoding.h"


static const char *lxb_filepath_test;


TEST_BEGIN(decode)
{
    lxb_char_t *buf, *end;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_JP);
    test_ne(enc_data, NULL);

    /* UTF-8: \x58; Unicode: \x00\x58; Code point: 88 */
    to_update_buffer("\x58");
    test_eq(test_decode_chunks(enc_data, buf, end), 88);
    test_eq(test_decode_full(enc_data, buf, end), 88);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x8D");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x90");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\xA0");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\xFF");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* UTF-8: \xEF\xBD\xA1; Unicode: \xFF\x61; Code point: 65377 */
    to_update_buffer("\x8E\xA1");
    test_eq(test_decode_chunks(enc_data, buf, end), 0xFF61);
    test_eq(test_decode_full(enc_data, buf, end), 0xFF61);

    /* UTF-8: \xEF\xBE\x9F; Unicode: \xFF\x9F; Code point: 65439 */
    to_update_buffer("\x8E\xDF");
    test_eq(test_decode_chunks(enc_data, buf, end), 0xFF9F);
    test_eq(test_decode_full(enc_data, buf, end), 0xFF9F);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x8E\xA0");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x8E\xE0");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x8F\xA1\xA1");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* UTF-8: \xCB\x98; Unicode: \x02\xD8; Code point: 728 */
    to_update_buffer("\x8F\xA2\xAF");
    test_eq(test_decode_chunks(enc_data, buf, end), 0x02D8);
    test_eq(test_decode_full(enc_data, buf, end), 0x02D8);

    /* UTF-8: \xE7\x92\xAF; Unicode: \x74\xAF; Code point: 29871 */
    to_update_buffer("\x8F\xCC\xE3");
    test_eq(test_decode_chunks(enc_data, buf, end), 0x74af);
    test_eq(test_decode_full(enc_data, buf, end), 0x74af);

    /* UTF-8: \xE7\x92\xAF; Unicode: \x9F\xA5; Code point: 40869 */
    to_update_buffer("\x8F\xED\xE3");
    test_eq(test_decode_chunks(enc_data, buf, end), 0x9FA5);
    test_eq(test_decode_full(enc_data, buf, end), 0x9FA5);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x8F\xFE\xFE");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* UTF-8: \xEF\xBC\x82; Unicode: \xFF\x02; Code point: 65282 */
    to_update_buffer("\xFC\xFE");
    test_eq(test_decode_chunks(enc_data, buf, end), 0xFF02);
    test_eq(test_decode_full(enc_data, buf, end), 0xFF02);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\xFE\xFE");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
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

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_JP);
    test_ne(enc_data, NULL);

    to_update_buffer("\xFF\xFC\xFE");
    test_buffer(test_decode_buffer_chunks, 2, rp_cp, 0xFF02);
    test_buffer(test_decode_buffer_full, 2, rp_cp, 0xFF02);

    to_update_buffer("\xFF\x8F\xA2\xAF");
    test_buffer(test_decode_buffer_chunks, 2, rp_cp, 0x02D8);
    test_buffer(test_decode_buffer_full, 2, rp_cp, 0x02D8);

    to_update_buffer("\x8F\xA2\xFF\xAF");
    test_buffer(test_decode_buffer_chunks, 2, rp_cp, LXB_ENCODING_DECODE_CONTINUE);
    test_buffer(test_decode_buffer_full, 2, rp_cp, LXB_ENCODING_DECODE_CONTINUE);

    to_update_buffer("\xA2\x32\xFC\xFE");
    test_buffer(test_decode_buffer_chunks, 3, rp_cp, 0x32, 0xFF02);
    test_buffer(test_decode_buffer_full, 3, rp_cp, 0x32, 0xFF02);

    to_update_buffer("\x8F\xED\x32\xFC\xFE");
    test_buffer(test_decode_buffer_chunks, 3, rp_cp, 0x32, 0xFF02);
    test_buffer(test_decode_buffer_full, 3, rp_cp, 0x32, 0xFF02);
}
TEST_END

static lxb_status_t
test_decode_process_file(const lxb_test_entry_t *entry, void *ctx, size_t line)
{
    const lxb_char_t *data, *end;
    lxb_codepoint_t cp;

    lxb_encoding_decode_t enc_ctx = {0};
    const lxb_encoding_data_t *enc_data = ctx;

    data = entry->data;
    end = data + entry->size;

    cp = enc_data->decode(&enc_ctx, &data, end);
    if (cp != entry->cp) {
        TEST_PRINTLN("Need: %u; Have: %u; Line: "LEXBOR_FORMAT_Z,
                     entry->cp, cp, line);

        TEST_PRINT("Bytes: ");
        for (size_t j = 0; j < entry->size; j++) {
            TEST_PRINT("\\x%02X", entry->data[j]);
        }

        TEST_PRINT("\n");

        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

TEST_BEGIN(decode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_JP);

    status = test_encoding_process_file(lxb_filepath_test, test_decode_process_file,
                                        (void *) enc_data, &line);
    if (status != LXB_STATUS_OK) {
        failed_and_exit(line);
    }
}
TEST_END

static lxb_status_t
test_encode_process_file(const lxb_test_entry_t *entry, void *ctx, size_t line)
{
    int8_t size, j;
    lxb_char_t data[8];
    lxb_char_t *ref, *end;
    lxb_encoding_encode_t enc_ctx = {0};
    const lxb_encoding_data_t *enc_data = ctx;

    ref = data;
    end = data + sizeof(data) - 1;

    size = enc_data->encode(&enc_ctx, &ref, end, entry->cp);
    if (size != entry->size
        || strncmp((const char *) data, (const char *) entry->data, size))
    {
        TEST_PRINT("Need: ");
        for (j = 0; j < (int8_t) entry->size; j++) {
            TEST_PRINT("\\x%02X", entry->data[j]);
        }

        TEST_PRINT("\n");
        TEST_PRINT("Have: ");

        if (size < LXB_ENCODING_ENCODE_OK) {
            TEST_PRINT("<ERROR>");
        }
        else {
            for (j = 0; j < size; j++) {
                TEST_PRINT("\\x%02X", data[j]);
            }
        }

        TEST_PRINT("\n");

        TEST_PRINTLN("Codepoint: %u; Line: "LEXBOR_FORMAT_Z, entry->cp, line);

        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

TEST_BEGIN(encode_map)
{
    size_t line;
    lxb_status_t status;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_JP);

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

    enc_data = lxb_encoding_data(LXB_ENCODING_EUC_JP);

    /* 2 */
    ref = &ch1;
    size = enc_data->encode(&ctx, &ref, ref + 1, 0xFA1F);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode(&ctx, &ref, ref + 2, 0xFA1F);
    test_eq(size, 2);
}
TEST_END

int
main(int argc, const char * argv[])
{
    if (argc != 2) {
        printf("Usage:\n\teuc_jp <filepath>\n");
        return EXIT_FAILURE;
    }

    lxb_filepath_test = argv[1];

    TEST_INIT();

    TEST_ADD(decode);
    TEST_ADD(decode_prepend);
    TEST_ADD(decode_map);
    TEST_ADD(encode_map);
    TEST_ADD(encode_buffer_check);

    TEST_RUN("lexbor/encoding/euc_jp");
    TEST_RELEASE();
}
