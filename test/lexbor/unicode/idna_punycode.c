/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/unicode/unicode.h>
#include <lexbor/encoding/encoding.h>


typedef struct {
    lexbor_str_t input;
    lexbor_str_t ascii;
    lexbor_str_t unicode;
}
test_idna_entry_t;

typedef struct {
    lxb_char_t data[128];
    size_t     length;
    size_t     calls;
}
test_idna_ctx_t;


static const test_idna_entry_t invalid_entries[] = {
    {lexbor_str("xn--"), {NULL, 0}, {NULL, 0}},
    {lexbor_str("xn--.a"), {NULL, 0}, {NULL, 0}},
    {lexbor_str("a.xn--"), {NULL, 0}, {NULL, 0}},
    {lexbor_str("xn--a-"), {NULL, 0}, {NULL, 0}},
    {lexbor_str("XN--A-"), {NULL, 0}, {NULL, 0}},
    {lexbor_str("xn--127-.0.0.1"), {NULL, 0}, {NULL, 0}},
    {lexbor_str("a.xn--b-"), {NULL, 0}, {NULL, 0}},
    {lexbor_str("xn--a-.xn--zca"), {NULL, 0}, {NULL, 0}},
    {lexbor_str("xn--zca.xn--a-"), {NULL, 0}, {NULL, 0}},
    {lexbor_str("\xC3\xA4.xn--a-"), {NULL, 0}, {NULL, 0}}
};

static const test_idna_entry_t valid_entries[] = {
    {
        lexbor_str("xn--zca"),
        lexbor_str("xn--zca"),
        lexbor_str("\xC3\x9F")
    },
    {
        lexbor_str("XN--ZCA.a"),
        lexbor_str("xn--zca.a"),
        lexbor_str("\xC3\x9F.a")
    },
    {
        lexbor_str("xn--a-0fa"),
        lexbor_str("xn--a-0fa"),
        lexbor_str("a\xC3\xA4")
    },
    {
        lexbor_str("xn--127-sla.0.0.1"),
        lexbor_str("xn--127-sla.0.0.1"),
        lexbor_str("127\xC3\xA4.0.0.1")
    }
};


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    test_idna_ctx_t *buf = ctx;

    buf->calls++;

    if (len > sizeof(buf->data) - buf->length) {
        return LXB_STATUS_ERROR_SMALL_BUFFER;
    }

    memcpy(buf->data + buf->length, data, len);

    buf->length += len;

    return LXB_STATUS_OK;
}

TEST_BEGIN_ARGS(test_convert, const test_idna_entry_t *entry, bool to_ascii)
{
    size_t i, cp_len;
    lxb_status_t status;
    lxb_codepoint_t cps[64];
    const lexbor_str_t *expected;
    const lxb_char_t *p, *end;
    lxb_unicode_idna_t idna;
    test_idna_ctx_t buf;

    p = entry->input.data;
    end = p + entry->input.length;
    cp_len = 0;

    while (p < end) {
        cps[cp_len++] = lxb_encoding_decode_valid_utf_8_single(&p, end);
    }

    expected = (to_ascii) ? &entry->ascii : &entry->unicode;

    status = lxb_unicode_idna_init(&idna);
    test_eq(status, LXB_STATUS_OK);

    for (i = 0; i < 2; i++) {
        TEST_PRINTLN("Input: %.*s; to_ascii: %u; codepoint_input: "
                     LEXBOR_FORMAT_Z, (int) entry->input.length,
                     (const char *) entry->input.data, (unsigned) to_ascii, i);

        buf.length = 0;
        buf.calls = 0;

        if (to_ascii) {
            if (i == 0) {
                status = lxb_unicode_idna_to_ascii(&idna, entry->input.data,
                                                   entry->input.length,
                                                   callback, &buf, 0);
            }
            else {
                status = lxb_unicode_idna_to_ascii_cp(&idna, cps, cp_len,
                                                      callback, &buf, 0);
            }
        }
        else if (i == 0) {
            status = lxb_unicode_idna_to_unicode(&idna, entry->input.data,
                                                 entry->input.length,
                                                 callback, &buf, 0);
        }
        else {
            status = lxb_unicode_idna_to_unicode_cp(&idna, cps, cp_len,
                                                    callback, &buf, 0);
        }

        lxb_unicode_idna_clean(&idna);

        if (expected->data == NULL) {
            test_eq(status, LXB_STATUS_ERROR_UNEXPECTED_RESULT);
            test_eq_size(buf.calls, 0);
        }
        else {
            test_eq(status, LXB_STATUS_OK);
            test_eq_size(buf.calls, 1);
            test_eq_str_n(buf.data, buf.length,
                          expected->data, expected->length);
        }
    }

    (void) lxb_unicode_idna_destroy(&idna, false);
}
TEST_END

TEST_BEGIN(to_ascii_invalid)
{
    size_t i;

    for (i = 0; i < sizeof(invalid_entries) / sizeof(invalid_entries[0]); i++) {
        TEST_CALL_ARGS(test_convert, &invalid_entries[i], true);
    }
}
TEST_END

TEST_BEGIN(to_unicode_invalid)
{
    size_t i;

    for (i = 0; i < sizeof(invalid_entries) / sizeof(invalid_entries[0]); i++) {
        TEST_CALL_ARGS(test_convert, &invalid_entries[i], false);
    }
}
TEST_END

TEST_BEGIN(to_ascii_valid)
{
    size_t i;

    for (i = 0; i < sizeof(valid_entries) / sizeof(valid_entries[0]); i++) {
        TEST_CALL_ARGS(test_convert, &valid_entries[i], true);
    }
}
TEST_END

TEST_BEGIN(to_unicode_valid)
{
    size_t i;

    for (i = 0; i < sizeof(valid_entries) / sizeof(valid_entries[0]); i++) {
        TEST_CALL_ARGS(test_convert, &valid_entries[i], false);
    }
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(to_ascii_invalid);
    TEST_ADD(to_unicode_invalid);
    TEST_ADD(to_ascii_valid);
    TEST_ADD(to_unicode_valid);

    TEST_RUN("lexbor/unicode/idna_punycode");
    TEST_RELEASE();
}
