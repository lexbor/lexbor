/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/encoding/encoding.h>
#include <lexbor/encoding/encode.h>

#include "encoding.h"


static const char *lxb_filepath_test;


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
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x1B\x23");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    /* LXB_ENCODING_DECODE_ERROR */
    to_update_buffer("\x1B\x24\x40\x21\x1B");
    test_eq(test_decode_chunks(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);
    test_eq(test_decode_full(enc_data, buf, end), LXB_ENCODING_DECODE_ERROR);

    to_update_buffer("\x1B\x24\x40\x21\x10\x21\x21");
    test_buffer(test_decode_buffer_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_buffer_chunks, 1, 0x3000);
    test_buffer(test_decode_buffer_full, 1, 0x3000);

    to_update_buffer("\x1B\x24\x40\x20\x21\x21");
    test_buffer(test_decode_buffer_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x24\x40\x21\x20\x21\x21");
    test_buffer(test_decode_buffer_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x24\x40\x7E\x21");
    test_buffer(test_decode_buffer_chunks, 1, rp_cp);
    test_buffer(test_decode_buffer_full, 1, rp_cp);

    to_update_buffer("\x1B\x24\x40\x7E\x7E");
    test_buffer(test_decode_buffer_chunks, 1, rp_cp);
    test_buffer(test_decode_buffer_full, 1, rp_cp);

    to_update_buffer("\x1B\x23\x32");
    test_buffer(test_decode_buffer_chunks, 3, rp_cp, 0x23, 0x32);
    test_buffer(test_decode_buffer_full, 3, rp_cp, 0x23, 0x32);

    to_update_buffer("\x1B\x28\x42\x32");
    test_buffer(test_decode_buffer_chunks, 1, 0x32);
    test_buffer(test_decode_buffer_full, 1, 0x32);

    /* ASCII */
    to_update_buffer("\x32");
    test_buffer(test_decode_buffer_chunks, 1, 0x32);
    test_buffer(test_decode_buffer_full, 1, 0x32);

    to_update_buffer("\x1B\x28\x42\x0E");
    test_buffer(test_decode_buffer_chunks, 1, rp_cp);
    test_buffer(test_decode_buffer_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x42\x0F");
    test_buffer(test_decode_buffer_chunks, 1, rp_cp);
    test_buffer(test_decode_buffer_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x42\x0F\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_buffer_chunks, 3, rp_cp, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 3, rp_cp, rp_cp, 0x3000);

    /* Roman */
    to_update_buffer("\x1B\x28\x4A\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_buffer_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x4A\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_buffer_chunks, 3, rp_cp, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 3, rp_cp, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x4A\x5C\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_buffer_chunks, 3, 0xA5, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 3, 0xA5, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x4A\x5C");
    test_buffer(test_decode_buffer_chunks, 1, 0xA5);
    test_buffer(test_decode_buffer_full, 1, 0xA5);

    to_update_buffer("\x1B\x28\x4A\x7E");
    test_buffer(test_decode_buffer_chunks, 1, 0x203E);
    test_buffer(test_decode_buffer_full, 1, 0x203E);

    to_update_buffer("\x1B\x28\x4A\x32\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_buffer_chunks, 3, 0x32, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 3, 0x32, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x4A\x32");
    test_buffer(test_decode_buffer_chunks, 1, 0x32);
    test_buffer(test_decode_buffer_full, 1, 0x32);

    to_update_buffer("\x1B\x28\x4A\x0E");
    test_buffer(test_decode_buffer_chunks, 1, rp_cp);
    test_buffer(test_decode_buffer_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x4A\x0F");
    test_buffer(test_decode_buffer_chunks, 1, rp_cp);
    test_buffer(test_decode_buffer_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x4A\x0F\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_buffer_chunks, 3, rp_cp, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 3, rp_cp, rp_cp, 0x3000);

    /* katakana */
    to_update_buffer("\x1B\x28\x49\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_buffer_chunks, 2, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 2, rp_cp, 0x3000);

    to_update_buffer("\x1B\x28\x49\x21");
    test_buffer(test_decode_buffer_chunks, 1, 0xFF61);
    test_buffer(test_decode_buffer_full, 1, 0xFF61);

    to_update_buffer("\x1B\x28\x49\x20");
    test_buffer(test_decode_buffer_chunks, 1, rp_cp);
    test_buffer(test_decode_buffer_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x49\x5F");
    test_buffer(test_decode_buffer_chunks, 1, 0xFF9F);
    test_buffer(test_decode_buffer_full, 1, 0xFF9F);

    to_update_buffer("\x1B\x28\x49\x60");
    test_buffer(test_decode_buffer_chunks, 1, rp_cp);
    test_buffer(test_decode_buffer_full, 1, rp_cp);

    to_update_buffer("\x1B\x28\x49\x0F\x1B\x24\x40\x21\x1B\x24\x40\x21\x21");
    test_buffer(test_decode_buffer_chunks, 3, rp_cp, rp_cp, 0x3000);
    test_buffer(test_decode_buffer_full, 3, rp_cp, rp_cp, 0x3000);
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
    test_buffer(test_decode_buffer_chunks, 3, rp_cp, 0x26, 0x32);
    test_buffer(test_decode_buffer_full, 3, rp_cp, 0x26, 0x32);

    to_update_buffer("\x1B\x24\x32");
    test_buffer(test_decode_buffer_chunks, 3, rp_cp, 0x24, 0x32);
    test_buffer(test_decode_buffer_full, 3, rp_cp, 0x24, 0x32);
}
TEST_END

static lxb_status_t
test_decode_process_file(const lxb_test_entry_t *entry, void *ctx, size_t line)
{
    const lxb_char_t *data, *end;
    lxb_codepoint_t cp;

    lxb_encoding_decode_t enc_ctx = {0};
    const lxb_encoding_data_t *enc_data = ctx;

    if (entry->cp >= 0xFF61 && entry->cp <= 0xFF9F) {
        return LXB_STATUS_OK;
    }

    data = entry->data;
    end = data + entry->size;

    if (entry->cp == 0x2212) {
        cp = enc_data->decode(&enc_ctx, &data, end);
        if (cp == 0xFF0D) {
            cp = 0x2212;
        }
    }
    else {
        cp = enc_data->decode(&enc_ctx, &data, end);
    }

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

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

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
    lxb_char_t data[12];
    lxb_char_t *ref, *end;
    lxb_encoding_encode_t enc_ctx = {0};
    const lxb_encoding_data_t *enc_data = ctx;

    ref = data;
    end = data + sizeof(data) - 1;

    size = enc_data->encode(&enc_ctx, &ref, end, entry->cp);
    size += lxb_encoding_encode_iso_2022_jp_eof(&enc_ctx, &ref, 
                                                ref + (end - ref));

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

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

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
    lxb_char_t ch1, ch2[2], ch3[3], ch4[4], ch5[5];
    lxb_char_t *ref;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(LXB_ENCODING_ISO_2022_JP);

    /* 4 */
    ref = &ch1;
    size = enc_data->encode(&ctx, &ref, ref + 1, 0x00A5);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode(&ctx, &ref, ref + 2, 0x00A5);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch3;
    size = enc_data->encode(&ctx, &ref, ref + 3, 0x00A5);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch4;
    size = enc_data->encode(&ctx, &ref, ref + 4, 0x00A5);
    test_eq(size, 4);

    /* 4 */
    ref = &ch1;
    size = enc_data->encode(&ctx, &ref, ref + 1, 0xFF61);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch2;
    size = enc_data->encode(&ctx, &ref, ref + 2, 0xFF61);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch3;
    size = enc_data->encode(&ctx, &ref, ref + 3, 0xFF61);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch4;
    size = enc_data->encode(&ctx, &ref, ref + 4, 0xFF61);
    test_eq(size, LXB_ENCODING_ENCODE_SMALL_BUFFER);

    ref = ch5;
    size = enc_data->encode(&ctx, &ref, ref + 5, 0xFF61);
    test_eq(size, 5);
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
    TEST_ADD(decode_map);
    TEST_ADD(encode_map);
    TEST_ADD(encode_buffer_check);

    TEST_RUN("lexbor/encoding/iso_2022_jp");
    TEST_RELEASE();
}
