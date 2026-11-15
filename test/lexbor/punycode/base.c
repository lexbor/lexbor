/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/str.h>
#include <lexbor/encoding/encoding.h>
#include <lexbor/punycode/punycode.h>


#define TEST_LARGE_LENGTH 1000000


static lxb_codepoint_t large_source[TEST_LARGE_LENGTH];
static lxb_codepoint_t large_out[TEST_LARGE_LENGTH + 1];


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lexbor_str_t *str = ctx;

    str->data = lexbor_malloc(len + 1);
    if (str->data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    str->length = len;

    memcpy(str->data, data, len);

    str->data[len] = '\0';

    return LXB_STATUS_OK;
}

static lxb_status_t
callback_enc(const lxb_char_t *data, size_t len, void *ctx, bool unchanged)
{
    return callback(data, len, ctx);
}

static lxb_status_t
callback_cp(const lxb_codepoint_t *cps, size_t len, void *ctx)
{
    lxb_codepoint_t *out = ctx;
    size_t i = 0;

    while (i++ < len) {
        *out++ = *cps++;
    }

    *out++ = 0x0000;

    return LXB_STATUS_OK;
}

TEST_BEGIN(encode_chr)
{
    lxb_status_t status;
    lexbor_str_t str;

    const lexbor_str_t input = lexbor_str("лексбор");

    status = lxb_punycode_encode(input.data, input.length, callback_enc, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str(str.data, "90ahpcsme");

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(encode_big_buffer)
{
    size_t length;
    lexbor_str_t str;
    lxb_status_t status;
    lxb_char_t *p;
    lxb_char_t buffer[4096 * 4];

    p = buffer;
    length = sizeof(buffer);

    for (size_t i = 0; i < length; i += 2) {
        *p++ = 0xD1;
        *p++ = 0x91;
    }

    status = lxb_punycode_encode(buffer, length, callback_enc, &str);
    test_eq(status, LXB_STATUS_OK);

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(encode_big_buffer_edge)
{
    size_t length;
    lexbor_str_t str;
    lxb_status_t status;
    lxb_char_t *p;
    lxb_char_t buffer[4096 * 2];

    p = buffer;
    length = sizeof(buffer);

    for (size_t i = 0; i < length; i += 2) {
        *p++ = 0xD1;
        *p++ = 0x91;
    }

    status = lxb_punycode_encode(buffer, length, callback_enc, &str);
    test_eq(status, LXB_STATUS_OK);

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(encode_truncated_utf_8)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_char_t buffer[4096 * 2 + 1];

    memset(buffer, 'a', sizeof(buffer) - 1);

    buffer[sizeof(buffer) - 1] = 0xD1;

    status = lxb_punycode_encode(buffer, sizeof(buffer), callback_enc, &str);
    test_eq(status, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    test_eq(str.data, NULL);
}
TEST_END

TEST_BEGIN(encode_overflow)
{
    size_t i;
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_codepoint_t source[4001];
    lxb_char_t buffer[4004];

    for (i = 0; i < 4000; i++) {
        source[i] = 'a';
        buffer[i] = 'a';
    }

    source[4000] = 0x1061C1;

    memcpy(&buffer[4000], "\xF4\x86\x87\x81", 4);

    status = lxb_punycode_encode_cp(source, 4001, callback_enc, &str);
    test_eq(status, LXB_STATUS_ERROR_OVERFLOW);
    test_eq(str.data, NULL);

    status = lxb_punycode_encode(buffer, sizeof(buffer), callback_enc, &str);
    test_eq(status, LXB_STATUS_ERROR_OVERFLOW);
    test_eq(str.data, NULL);
}
TEST_END

TEST_BEGIN(encode_delta_max)
{
    size_t i;
    lxb_status_t status;
    lexbor_str_t str;
    lxb_codepoint_t source[4096];
    lxb_codepoint_t out[4097];

    for (i = 0; i < 4095; i++) {
        source[i] = 'a';
    }

    source[4095] = 0x10007F;

    status = lxb_punycode_encode_cp(source, 4096, callback_enc, &str);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_punycode_decode_cb_cp(str.data, str.length, callback_cp, out);
    test_eq(status, LXB_STATUS_OK);

    for (i = 0; i < 4096; i++) {
        test_eq(out[i], source[i]);
    }

    test_eq(out[4096], 0x0000);

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(encode_distinct)
{
    size_t i;
    lexbor_str_t str;
    lxb_status_t status;

    for (i = 0; i < TEST_LARGE_LENGTH; i++) {
        large_source[i] = 0x80 + i;
    }

    status = lxb_punycode_encode_cp(large_source, TEST_LARGE_LENGTH,
                                    callback_enc, &str);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_punycode_decode_cb_cp(str.data, str.length, callback_cp,
                                       large_out);
    test_eq(status, LXB_STATUS_OK);

    for (i = 0; i < TEST_LARGE_LENGTH; i++) {
        test_eq(large_out[i], large_source[i]);
    }

    test_eq(large_out[TEST_LARGE_LENGTH], 0x0000);

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(encode_mixed)
{
    size_t i;
    uint32_t seed;
    lexbor_str_t str;
    lxb_status_t status;
    lxb_codepoint_t source[10000];
    lxb_codepoint_t out[10001];

    seed = 1;

    for (i = 0; i < 10000; i++) {
        seed = seed * 1103515245 + 12345;

        switch ((seed >> 16) % 4) {
            case 0:
                source[i] = 'a' + (seed >> 8) % 26;
                break;

            case 1:
            case 2:
                source[i] = 0x0430 + (seed >> 8) % 16;
                break;

            default:
                source[i] = 0x80 + (seed >> 4) % 0x10FF80;
                break;
        }
    }

    status = lxb_punycode_encode_cp(source, 10000, callback_enc, &str);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_punycode_decode_cb_cp(str.data, str.length, callback_cp, out);
    test_eq(status, LXB_STATUS_OK);

    for (i = 0; i < 10000; i++) {
        test_eq(out[i], source[i]);
    }

    test_eq(out[10000], 0x0000);

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(decode_distinct)
{
    size_t i;
    lexbor_str_t str;
    lxb_status_t status;

    for (i = 0; i < TEST_LARGE_LENGTH; i++) {
        large_source[i] = 0x80 + (TEST_LARGE_LENGTH - 1 - i);
    }

    status = lxb_punycode_encode_cp(large_source, TEST_LARGE_LENGTH,
                                    callback_enc, &str);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_punycode_decode_cb_cp(str.data, str.length, callback_cp,
                                       large_out);
    test_eq(status, LXB_STATUS_OK);

    for (i = 0; i < TEST_LARGE_LENGTH; i++) {
        test_eq(large_out[i], large_source[i]);
    }

    test_eq(large_out[TEST_LARGE_LENGTH], 0x0000);

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(decode_mixed)
{
    size_t i, length;
    uint32_t seed;
    lexbor_str_t str;
    lxb_status_t status;
    lxb_codepoint_t *ace;

    length = 100000;
    seed = 1;

    for (i = 0; i < length; i++) {
        seed = seed * 1103515245 + 12345;

        switch ((seed >> 16) % 4) {
            case 0:
                large_source[i] = 'a' + (seed >> 8) % 26;
                break;

            case 1:
                large_source[i] = 0x0430 + (seed >> 8) % 16;
                break;

            default:
                large_source[i] = 0x80 + (seed >> 4) % 0x10FF80;
                break;
        }
    }

    status = lxb_punycode_encode_cp(large_source, length, callback_enc, &str);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_punycode_decode_cb_cp(str.data, str.length, callback_cp,
                                       large_out);
    test_eq(status, LXB_STATUS_OK);

    for (i = 0; i < length; i++) {
        test_eq(large_out[i], large_source[i]);
    }

    test_eq(large_out[length], 0x0000);

    ace = lexbor_malloc(str.length * sizeof(lxb_codepoint_t));
    test_ne(ace, NULL);

    for (i = 0; i < str.length; i++) {
        ace[i] = str.data[i];
    }

    memset(large_out, 0xFF, (length + 1) * sizeof(lxb_codepoint_t));

    status = lxb_punycode_decode_cp(ace, str.length, callback_cp, large_out);
    test_eq(status, LXB_STATUS_OK);

    for (i = 0; i < length; i++) {
        test_eq(large_out[i], large_source[i]);
    }

    test_eq(large_out[length], 0x0000);

    lexbor_free(ace);
    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(decode_chr)
{
    lxb_status_t status;
    lexbor_str_t str;

    const lexbor_str_t input = lexbor_str("90ahpcsme");

    status = lxb_punycode_decode(input.data, input.length, callback, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str(str.data, "лексбор");

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(decode_big_buffer)
{
    size_t length;
    lexbor_str_t str;
    lxb_status_t status;
    lxb_char_t *p;
    lxb_char_t buffer[4096 * 6];

    p = buffer;
    length = sizeof(buffer);

    for (size_t i = 0; i < length; i += 3) {
        *p++ = '6';
        *p++ = '1';
        *p++ = 'a';
    }

    status = lxb_punycode_decode(buffer, length, callback, &str);
    test_eq(status, LXB_STATUS_OK);

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(decode_big_buffer_edge)
{
    size_t length;
    lexbor_str_t str;
    lxb_status_t status;
    lxb_char_t *p;
    lxb_char_t buffer[4096 * 3];

    p = buffer;
    length = sizeof(buffer);

    for (size_t i = 0; i < length; i += 3) {
        *p++ = '6';
        *p++ = '1';
        *p++ = 'a';
    }

    status = lxb_punycode_decode(buffer, length, callback, &str);
    test_eq(status, LXB_STATUS_OK);

    lexbor_free(str.data);
}
TEST_END

TEST_BEGIN(cp)
{
    lxb_status_t status;
    lexbor_str_t str;
    const lxb_char_t *input_p, *input_end;
    lxb_codepoint_t *p, *end, *out_p;
    lxb_codepoint_t source[4096];
    lxb_codepoint_t out[4096];

    const lexbor_str_t input = lexbor_str("лексбор");

    input_p = input.data;
    input_end = input_p + input.length;

    p = source;

    while (input_p < input_end) {
        *p++ = lxb_encoding_decode_valid_utf_8_single(&input_p, input_end);
    }

    status = lxb_punycode_encode_cp(source, p - source, callback_enc, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str(str.data, "90ahpcsme");

    status = lxb_punycode_decode_cb_cp(str.data, str.length, callback_cp, out);
    test_eq(status, LXB_STATUS_OK);

    end = p;
    p = source;
    out_p = out;

    while (p < end) {
        test_eq(*p, *out_p);

        p += 1;
        out_p += 1;
    }

    test_eq(*out_p, 0x0000);

    lexbor_free(str.data);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(encode_chr);
    TEST_ADD(encode_big_buffer);
    TEST_ADD(encode_big_buffer_edge);
    TEST_ADD(encode_truncated_utf_8);
    TEST_ADD(encode_overflow);
    TEST_ADD(encode_delta_max);
    TEST_ADD(encode_distinct);
    TEST_ADD(encode_mixed);
    TEST_ADD(decode_distinct);
    TEST_ADD(decode_mixed);
    TEST_ADD(decode_chr);
    TEST_ADD(decode_big_buffer);
    TEST_ADD(decode_big_buffer_edge);
    TEST_ADD(cp);

    TEST_RUN("lexbor/punycode/base");
    TEST_RELEASE();
}
