/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <inttypes.h>
#include <unit/test.h>

#include <lexbor/unicode/unicode.h>
#include <lexbor/encoding/encoding.h>


typedef struct {
    lxb_codepoint_t cps[4096];
    lxb_codepoint_t *p;
}
test_unicode_ctx_t;


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


TEST_BEGIN(chunks)
{
    size_t length;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;
    test_unicode_ctx_t test_ctx;

    lxb_char_t source[] = "\u1E9B\u0323";

    test_ctx.p = test_ctx.cps;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, LXB_UNICODE_NFC);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_unicode_normalize(uc, source, 2, callback, &test_ctx, false);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_unicode_normalize(uc, source + 2, sizeof(source) - 3,
                                   callback, &test_ctx, true);
    test_eq(status, LXB_STATUS_OK);

    length = test_ctx.p - test_ctx.cps;
    test_eq(length, 2);

    test_eq(test_ctx.cps[0], 0x1e9b);
    test_eq(test_ctx.cps[1], 0x0323);

    (void) lxb_unicode_normalizer_destroy(uc, true);
}
TEST_END

TEST_BEGIN(edge)
{
    size_t length;
    lxb_char_t *p, *end;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;
    lxb_codepoint_t cp, *cpd;
    test_unicode_ctx_t test_ctx;

    /* Starter. */
    lxb_char_t source[4096] = "\u04D6";

    p = source + 2;

    for (size_t i = 0; i < 1024; i++) {
        /* \u0300 */
        *p++ = 0xcc;
        *p++ = 0x80;
    }

    /* Starter. */
    /* \u0415 */
    *p++ = 0xd0;
    *p++ = 0x95;

    /* Ending. */
    *p++ = 'a';
    *p++ = 'b';
    *p++ = 'c';

    test_ctx.p = test_ctx.cps;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, LXB_UNICODE_NFC);
    test_eq(status, LXB_STATUS_OK);

    lxb_unicode_flush_count_set(uc, 1024);

    status = lxb_unicode_normalize(uc, source, p - source,
                                   callback, &test_ctx, true);
    test_eq(status, LXB_STATUS_OK);

    length = test_ctx.p - test_ctx.cps;
    test_eq(length, 1029);

    end = p;
    p = source;
    cpd = test_ctx.cps;

    while (p < end) {
        cp = lxb_encoding_decode_valid_utf_8_single((const lxb_char_t **) &p,
                                                    end);
        test_eq(*cpd, cp);

        cpd += 1;
    }

    (void) lxb_unicode_normalizer_destroy(uc, true);
}
TEST_END

TEST_BEGIN(one_by_one)
{
    size_t length;
    lxb_char_t *p, *end;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;
    lxb_codepoint_t cp, *cpd;
    test_unicode_ctx_t test_ctx;

    /* Starter. */
    lxb_char_t source[4096] = "\u04D6";

    p = source + 2;

    for (size_t i = 0; i < 1024; i++) {
        /* \u0300 */
        *p++ = 0xcc;
        *p++ = 0x80;
    }

    /* Starter. */
    /* \u0415 */
    *p++ = 0xd0;
    *p++ = 0x95;

    /* Ending. */
    *p++ = 'a';
    *p++ = 'b';
    *p++ = 'c';

    test_ctx.p = test_ctx.cps;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, LXB_UNICODE_NFC);
    test_eq(status, LXB_STATUS_OK);

    lxb_unicode_flush_count_set(uc, 1024);

    end = p;
    p = source;

    while (p < end) {
        status = lxb_unicode_normalize(uc, p, 1, callback, &test_ctx, false);
        test_eq(status, LXB_STATUS_OK);

        p += 1;
    }

    status = lxb_unicode_normalize_end(uc, callback, &test_ctx);
    test_eq(status, LXB_STATUS_OK);

    length = test_ctx.p - test_ctx.cps;
    test_eq(length, 1029);

    end = p;
    p = source;
    cpd = test_ctx.cps;

    while (p < end) {
        cp = lxb_encoding_decode_valid_utf_8_single((const lxb_char_t **) &p,
                                                    end);
        test_eq(*cpd, cp);

        cpd += 1;
    }

    (void) lxb_unicode_normalizer_destroy(uc, true);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(chunks);
    TEST_ADD(edge);
    TEST_ADD(one_by_one);

    TEST_RUN("lexbor/unicode/edges_normalization_forms");
    TEST_RELEASE();
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lxb_codepoint_t cp;
    test_unicode_ctx_t *test_ctx;
    const lxb_char_t *p, *end;

    p = data;
    end = data + len;

    test_ctx = ctx;

    while (p < end) {
        cp = lxb_encoding_decode_valid_utf_8_single(&p, end);

        *test_ctx->p++ = cp;
    }

    return LXB_STATUS_OK;
}
