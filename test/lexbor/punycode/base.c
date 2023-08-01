/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/str.h>
#include <lexbor/encoding/encoding.h>
#include <lexbor/punycode/punycode.h>


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
    TEST_ADD(decode_chr);
    TEST_ADD(decode_big_buffer);
    TEST_ADD(decode_big_buffer_edge);
    TEST_ADD(cp);

    TEST_RUN("lexbor/punycode/base");
    TEST_RELEASE();
}
