/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/encoding/encoding.h>


typedef struct {
    lxb_encoding_t  encoding;
    size_t          length;
    lxb_char_t      data[8];
    size_t          cps_length;
    lxb_codepoint_t cps[4];
}
empty_entry_t;


static const empty_entry_t entries[] = {
    {LXB_ENCODING_BIG5, 2, {0xA4, 0x40}, 1, {0x4E00}},
    {LXB_ENCODING_BIG5, 3, {0x88, 0x62, 0x41}, 3, {0x00CA, 0x0304, 0x41}},
    {LXB_ENCODING_EUC_JP, 2, {0xA4, 0xA2}, 1, {0x3042}},
    {LXB_ENCODING_EUC_JP, 2, {0x8E, 0xB1}, 1, {0xFF71}},
    {LXB_ENCODING_EUC_JP, 3, {0x8F, 0xA2, 0xAF}, 1, {0x02D8}},
    {LXB_ENCODING_EUC_KR, 2, {0xB0, 0xA1}, 1, {0xAC00}},
    {LXB_ENCODING_SHIFT_JIS, 2, {0x82, 0xA0}, 1, {0x3042}},
    {LXB_ENCODING_SHIFT_JIS, 2, {0x81, 0x20}, 2, {LXB_ENCODING_DECODE_ERROR, 0x20}},
    {LXB_ENCODING_GB18030, 2, {0xB0, 0xA1}, 1, {0x554A}},
    {LXB_ENCODING_GB18030, 4, {0x81, 0x30, 0x81, 0x30}, 1, {0x0080}},
    {LXB_ENCODING_GB18030, 5, {0x81, 0x30, 0x81, 0x41, 0x42}, 4, {LXB_ENCODING_DECODE_ERROR, 0x30, 0x4E04, 0x42}},
    {LXB_ENCODING_GB18030, 3, {0x81, 0x30, 0x41}, 3, {LXB_ENCODING_DECODE_ERROR, 0x30, 0x41}},
    {LXB_ENCODING_GBK, 4, {0x81, 0x30, 0x81, 0x30}, 1, {0x0080}},
    {LXB_ENCODING_UTF_16LE, 2, {0x42, 0x30}, 1, {0x3042}},
    {LXB_ENCODING_UTF_16LE, 4, {0x3D, 0xD8, 0x00, 0xDE}, 1, {0x1F600}},
    {LXB_ENCODING_UTF_16LE, 4, {0x3D, 0xD8, 0x41, 0x00}, 2, {LXB_ENCODING_DECODE_ERROR, 0x41}},
    {LXB_ENCODING_UTF_16BE, 4, {0xD8, 0x3D, 0xDE, 0x00}, 1, {0x1F600}},
    {LXB_ENCODING_UTF_8, 3, {0xE3, 0x81, 0x82}, 1, {0x3042}},
    {LXB_ENCODING_UTF_8, 4, {0xF0, 0x9F, 0x98, 0x80}, 1, {0x1F600}},
    {LXB_ENCODING_ISO_2022_JP, 5, {0x1B, 0x24, 0x42, 0x24, 0x22}, 1, {0x3042}},
    {LXB_ENCODING_ISO_2022_JP, 4, {0x1B, 0x28, 0x5A, 0x41}, 4, {LXB_ENCODING_DECODE_ERROR, 0x28, 0x5A, 0x41}},
};


static bool
decode_chunk(const lxb_encoding_data_t *enc_data, lxb_encoding_decode_t *ctx,
             const lxb_char_t *p, const lxb_char_t *end,
             lxb_codepoint_t *cps, size_t *count, size_t max)
{
    lxb_codepoint_t cp;
    const lxb_char_t *tmp;

    while (p < end) {
        tmp = p;
        cp = enc_data->decode_single(ctx, &tmp, p);

        if (cp != LXB_ENCODING_DECODE_CONTINUE || tmp != p) {
            return false;
        }

        cp = enc_data->decode_single(ctx, &p, end);

        if (cp == LXB_ENCODING_DECODE_CONTINUE) {
            continue;
        }

        if (*count >= max) {
            return false;
        }

        cps[(*count)++] = cp;
    }

    tmp = end;
    cp = enc_data->decode_single(ctx, &tmp, end);

    return cp == LXB_ENCODING_DECODE_CONTINUE && tmp == end && p == end;
}

TEST_BEGIN(decode_single_empty)
{
    lxb_encoding_t id;
    lxb_codepoint_t cp;
    lxb_encoding_decode_t ctx;
    const lxb_char_t dummy = 0x41;
    const lxb_char_t *p;
    const lxb_encoding_data_t *enc_data;

    for (id = 0; id < LXB_ENCODING_LAST_ENTRY; id++) {
        enc_data = lxb_encoding_data(id);
        test_ne(enc_data, NULL);
        test_ne(enc_data->decode_single, NULL);

        test_eq(lxb_encoding_decode_init_single(&ctx, enc_data),
                LXB_STATUS_OK);

        p = &dummy;
        cp = enc_data->decode_single(&ctx, &p, p);

        if (id == LXB_ENCODING_AUTO || id == LXB_ENCODING_UNDEFINED
            || id == LXB_ENCODING_REPLACEMENT)
        {
            test_eq(cp, LXB_ENCODING_DECODE_ERROR);
        }
        else {
            test_eq(cp, LXB_ENCODING_DECODE_CONTINUE);
        }

        test_eq(p, &dummy);
    }
}
TEST_END

TEST_BEGIN(decode_single_empty_pending)
{
    size_t i, split, count;
    lxb_char_t *first;
    lxb_codepoint_t cps[4];
    lxb_encoding_decode_t ctx;
    const empty_entry_t *entry;
    const lxb_encoding_data_t *enc_data;

    for (i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        entry = &entries[i];

        enc_data = lxb_encoding_data(entry->encoding);
        test_ne(enc_data, NULL);

        for (split = 1; split <= entry->length; split++) {
            first = lexbor_malloc(split);
            test_ne(first, NULL);

            memcpy(first, entry->data, split);

            test_eq(lxb_encoding_decode_init_single(&ctx, enc_data),
                    LXB_STATUS_OK);

            count = 0;

            test_eq(decode_chunk(enc_data, &ctx, first, first + split,
                                 cps, &count, 4), true);
            test_eq(decode_chunk(enc_data, &ctx, entry->data + split,
                                 entry->data + entry->length,
                                 cps, &count, 4), true);

            test_eq(count, entry->cps_length);
            test_eq(memcmp(cps, entry->cps, count * sizeof(lxb_codepoint_t)),
                    0);

            lexbor_free(first);
        }
    }
}
TEST_END

TEST_BEGIN(valid_utf_8_empty)
{
    lxb_codepoint_t cp;
    const lxb_char_t dummy = 0x41;
    const lxb_char_t *p = &dummy;

    cp = lxb_encoding_decode_valid_utf_8_single(&p, p);
    test_eq(cp, LXB_ENCODING_DECODE_ERROR);
    test_eq(p, &dummy);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(decode_single_empty);
    TEST_ADD(decode_single_empty_pending);
    TEST_ADD(valid_utf_8_empty);

    TEST_RUN("lexbor/encoding/empty_slice");
    TEST_RELEASE();
}
