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
    lxb_codepoint_t cps[8192];
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

/*
 * The flush encodes into a fixed buffer. Fill it to the last byte, then
 * an ASCII code point: it must go to the next chunk, not past the buffer.
 */
static lxb_status_t
flush_full(size_t count, size_t flush_count, const lxb_char_t *cp_bytes,
           size_t cp_len)
{
    size_t i, f, length;
    lxb_char_t *source, *p, *end;
    lxb_status_t status;
    lxb_codepoint_t cp, *cpd;
    lxb_unicode_normalizer_t *uc;
    test_unicode_ctx_t test_ctx;

    static const lxb_unicode_form_t forms[] = {
        LXB_UNICODE_NFC, LXB_UNICODE_NFD, LXB_UNICODE_NFKC, LXB_UNICODE_NFKD
    };

    source = lexbor_malloc(count * cp_len + 1);
    if (source == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    p = source;

    for (i = 0; i < count; i++) {
        memcpy(p, cp_bytes, cp_len);
        p += cp_len;
    }

    *p++ = 'A';
    end = p;

    status = LXB_STATUS_ERROR;

    for (f = 0; f < sizeof(forms) / sizeof(forms[0]); f++) {
        uc = lxb_unicode_normalizer_create();
        status = lxb_unicode_normalizer_init(uc, forms[f]);
        if (status != LXB_STATUS_OK) {
            goto failed;
        }

        if (flush_count != 0) {
            lxb_unicode_flush_count_set(uc, flush_count);
        }

        test_ctx.p = test_ctx.cps;

        status = lxb_unicode_normalize(uc, source, end - source,
                                       callback, &test_ctx, true);
        if (status != LXB_STATUS_OK) {
            goto failed;
        }

        status = LXB_STATUS_ERROR;

        length = test_ctx.p - test_ctx.cps;
        if (length != count + 1) {
            goto failed;
        }

        p = source;
        cpd = test_ctx.cps;

        while (p < end) {
            cp = lxb_encoding_decode_valid_utf_8_single((const lxb_char_t **) &p,
                                                        end);
            if (*cpd++ != cp) {
                goto failed;
            }
        }

        status = LXB_STATUS_OK;

        (void) lxb_unicode_normalizer_destroy(uc, true);
    }

    lexbor_free(source);

    return status;

failed:

    (void) lxb_unicode_normalizer_destroy(uc, true);
    lexbor_free(source);

    return status;
}

TEST_BEGIN(flush_compaction_hangul_tail)
{
    size_t length;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;
    test_unicode_ctx_t test_ctx;

    const lxb_char_t source[] = "a\xEA\xB0\x80";

    test_ctx.p = test_ctx.cps;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, LXB_UNICODE_NFD);
    test_eq(status, LXB_STATUS_OK);

    lxb_unicode_flush_count_set(uc, 1);

    status = lxb_unicode_normalize(uc, source, sizeof(source) - 1,
                                   callback, &test_ctx, true);
    test_eq(status, LXB_STATUS_OK);

    length = test_ctx.p - test_ctx.cps;
    test_eq(length, 3);

    test_eq(test_ctx.cps[0], 0x0061);
    test_eq(test_ctx.cps[1], 0x1100);
    test_eq(test_ctx.cps[2], 0x1161);

    (void) lxb_unicode_normalizer_destroy(uc, true);
}
TEST_END

TEST_BEGIN(flush_compaction_mark_tail)
{
    size_t length;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;
    test_unicode_ctx_t test_ctx;

    const lxb_char_t source[] = "b\xC3\x80";

    test_ctx.p = test_ctx.cps;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, LXB_UNICODE_NFD);
    test_eq(status, LXB_STATUS_OK);

    lxb_unicode_flush_count_set(uc, 1);

    status = lxb_unicode_normalize(uc, source, sizeof(source) - 1,
                                   callback, &test_ctx, true);
    test_eq(status, LXB_STATUS_OK);

    length = test_ctx.p - test_ctx.cps;
    test_eq(length, 3);

    test_eq(test_ctx.cps[0], 0x0062);
    test_eq(test_ctx.cps[1], 0x0041);
    test_eq(test_ctx.cps[2], 0x0300);

    (void) lxb_unicode_normalizer_destroy(uc, true);
}
TEST_END

TEST_BEGIN(flush_full_buffer)
{
    /* U+0301, ccc 230: the first starter comes after 4096 bytes. */
    test_eq(flush_full(2048, 0, (const lxb_char_t *) "\xCC\x81", 2),
            LXB_STATUS_OK);

    /* U+1F600, starter: 1024 * 4 bytes, then 'A' in the same flush. */
    test_eq(flush_full(1024, 1025, (const lxb_char_t *) "\xF0\x9F\x98\x80", 4),
            LXB_STATUS_OK);

    test_eq(flush_full(1024, 4096, (const lxb_char_t *) "\xF0\x9F\x98\x80", 4),
            LXB_STATUS_OK);

    /* U+0800, 3 bytes: 1365 * 3 = 4095, one byte left for 'A'. */
    test_eq(flush_full(1365, 2048, (const lxb_char_t *) "\xE0\xA0\x80", 3),
            LXB_STATUS_OK);
}
TEST_END

static lxb_status_t
normalize_chunks(lxb_unicode_form_t form, const lxb_char_t *data,
                 size_t length, size_t chunk, test_unicode_ctx_t *test_ctx)
{
    size_t i;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;

    test_ctx->p = test_ctx->cps;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, form);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    if (chunk == 0) {
        status = lxb_unicode_normalize(uc, data, length, callback, test_ctx,
                                       true);
        goto done;
    }

    for (i = 0; i < length; i += chunk) {
        status = lxb_unicode_normalize(uc, data + i,
                                       lexbor_min(chunk, length - i),
                                       callback, test_ctx, false);
        if (status != LXB_STATUS_OK) {
            goto done;
        }
    }

    status = lxb_unicode_normalize_end(uc, callback, test_ctx);

done:

    (void) lxb_unicode_normalizer_destroy(uc, true);

    return status;
}

TEST_BEGIN_ARGS(test_chunks_equal, const lxb_char_t *data, size_t length)
{
    size_t f, chunk, len;
    lxb_status_t status;
    test_unicode_ctx_t whole, chunked;

    static const lxb_unicode_form_t forms[] = {
        LXB_UNICODE_NFC, LXB_UNICODE_NFD, LXB_UNICODE_NFKC, LXB_UNICODE_NFKD
    };

    for (f = 0; f < sizeof(forms) / sizeof(forms[0]); f++) {
        status = normalize_chunks(forms[f], data, length, 0, &whole);
        test_eq(status, LXB_STATUS_OK);

        len = whole.p - whole.cps;

        for (chunk = 1; chunk <= length; chunk++) {
            TEST_PRINTLN("Form: %d; chunk: "LEXBOR_FORMAT_Z, (int) forms[f],
                         chunk);

            status = normalize_chunks(forms[f], data, length, chunk, &chunked);
            test_eq(status, LXB_STATUS_OK);

            test_eq_size((chunked.p - chunked.cps), len);
            test_eq(memcmp(chunked.cps, whole.cps,
                           len * sizeof(lxb_codepoint_t)), 0);
        }
    }
}
TEST_END

TEST_BEGIN(utf_8_split_sequences)
{
    static const lxb_char_t source[] = "aéḊ \U0001F600́"
                                       "가Ḋ\U0001F601b";

    TEST_CALL_ARGS(test_chunks_equal, source, sizeof(source) - 1);
}
TEST_END

TEST_BEGIN(utf_8_incomplete_end)
{
    size_t length;
    lxb_status_t status;
    test_unicode_ctx_t test_ctx;

    static const lxb_char_t source[] = "abc\xE1\xB8";

    TEST_CALL_ARGS(test_chunks_equal, source, sizeof(source) - 1);
    TEST_CALL_ARGS(test_chunks_equal, source, sizeof(source) - 2);
    TEST_CALL_ARGS(test_chunks_equal, (const lxb_char_t *) "\xF0\x9F\x98", 3);

    status = normalize_chunks(LXB_UNICODE_NFC, source, sizeof(source) - 2, 4,
                              &test_ctx);
    test_eq(status, LXB_STATUS_OK);

    length = test_ctx.p - test_ctx.cps;
    test_eq_size(length, 4);

    test_eq(test_ctx.cps[0], 'a');
    test_eq(test_ctx.cps[1], 'b');
    test_eq(test_ctx.cps[2], 'c');
    test_eq(test_ctx.cps[3], LXB_ENCODING_REPLACEMENT_CODEPOINT);
}
TEST_END

TEST_BEGIN(utf_8_invalid_chunk_end)
{
    static const lxb_char_t source[] = "a\x80" "bcd\xBF\xE1\xB8\x8A";

    TEST_CALL_ARGS(test_chunks_equal, source, sizeof(source) - 1);
}
TEST_END

TEST_BEGIN(quick_check_incomplete_end)
{
    bool need;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, LXB_UNICODE_NFD);
    test_eq(status, LXB_STATUS_OK);

    need = lxb_unicode_quick_check(uc, (const lxb_char_t *) "\xCC\x81\xE1",
                                   3, false);
    test_eq(need, false);

    need = lxb_unicode_quick_check_end(uc);
    test_eq(need, false);

    need = lxb_unicode_quick_check(uc, (const lxb_char_t *) "\xCC\x96",
                                   2, true);
    test_eq(need, false);

    (void) lxb_unicode_normalizer_destroy(uc, true);
}
TEST_END

static lxb_status_t
normalize_flush(lxb_unicode_form_t form, const lxb_char_t *data,
                size_t length, size_t flush_count,
                test_unicode_ctx_t *test_ctx)
{
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;

    test_ctx->p = test_ctx->cps;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, form);
    if (status == LXB_STATUS_OK) {
        lxb_unicode_flush_count_set(uc, flush_count);

        status = lxb_unicode_normalize(uc, data, length, callback, test_ctx,
                                       true);
    }

    (void) lxb_unicode_normalizer_destroy(uc, true);

    return status;
}

TEST_BEGIN_ARGS(test_flush_equal, const lxb_char_t *data, size_t length)
{
    size_t f, i, len;
    lxb_status_t status;
    test_unicode_ctx_t whole, flushed;

    static const lxb_unicode_form_t forms[] = {
        LXB_UNICODE_NFC, LXB_UNICODE_NFD, LXB_UNICODE_NFKC, LXB_UNICODE_NFKD
    };

    static const size_t counts[] = {0, 1, 2, 3, 1023, 1024, 1025};

    for (f = 0; f < sizeof(forms) / sizeof(forms[0]); f++) {
        status = normalize_flush(forms[f], data, length, SIZE_MAX, &whole);
        test_eq(status, LXB_STATUS_OK);

        len = whole.p - whole.cps;

        for (i = 0; i < sizeof(counts) / sizeof(counts[0]); i++) {
            TEST_PRINTLN("Form: %d; flush count: "LEXBOR_FORMAT_Z,
                         (int) forms[f], counts[i]);

            status = normalize_flush(forms[f], data, length, counts[i],
                                     &flushed);
            test_eq(status, LXB_STATUS_OK);

            test_eq_size((flushed.p - flushed.cps), len);
            test_eq(memcmp(flushed.cps, whole.cps,
                           len * sizeof(lxb_codepoint_t)), 0);
        }
    }
}
TEST_END

TEST_BEGIN(flush_decomposition)
{
    size_t i, length;
    lxb_status_t status;
    test_unicode_ctx_t test_ctx;
    lxb_char_t source[1024 + 8];

    static const lxb_char_t mixed[] = "ᄀ각ẛ̣Å"
                                      "ﬁཱི̈́각éx";

    memset(source, 'a', 1024);
    memcpy(source + 1024, "\xC3\xA9" "bcd", 5);

    status = normalize_flush(LXB_UNICODE_NFD, source, 1024 + 5, 1024,
                             &test_ctx);
    test_eq(status, LXB_STATUS_OK);

    length = test_ctx.p - test_ctx.cps;
    test_eq_size(length, 1024 + 5);

    for (i = 0; i < 1024; i++) {
        test_eq(test_ctx.cps[i], 'a');
    }

    test_eq(test_ctx.cps[1024], 'e');
    test_eq(test_ctx.cps[1025], 0x0301);
    test_eq(test_ctx.cps[1026], 'b');

    TEST_CALL_ARGS(test_flush_equal, source, 1024 + 5);

    memcpy(source + 1024, "\xEA\xB0\x81" "b", 4);

    TEST_CALL_ARGS(test_flush_equal, source, 1024 + 4);
    TEST_CALL_ARGS(test_flush_equal, mixed, sizeof(mixed) - 1);
}
TEST_END

static lxb_status_t
callback_flushed(const lxb_char_t *data, size_t len, void *ctx)
{
    const lxb_char_t *end;
    test_unicode_ctx_t *test_ctx = ctx;

    end = data + len;

    while (data < end) {
        if (test_ctx->p == test_ctx->cps + (sizeof(test_ctx->cps)
                                            / sizeof(lxb_codepoint_t)))
        {
            return LXB_STATUS_ERROR_SMALL_BUFFER;
        }

        *test_ctx->p++ = lxb_encoding_decode_valid_utf_8_single(&data, end);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
callback_cp(const lxb_codepoint_t *cps, size_t len, void *ctx)
{
    test_unicode_ctx_t *test_ctx = ctx;

    if (len > lxb_size(test_ctx->cps + (sizeof(test_ctx->cps)
                                        / sizeof(lxb_codepoint_t))
                       - test_ctx->p))
    {
        return LXB_STATUS_ERROR_SMALL_BUFFER;
    }

    memcpy(test_ctx->p, cps, len * sizeof(lxb_codepoint_t));

    test_ctx->p += len;

    return LXB_STATUS_OK;
}

static lxb_status_t
normalize_flushed(lxb_unicode_form_t form, const lxb_char_t *data,
                  size_t length, size_t chunk, size_t flush_count,
                  test_unicode_ctx_t *test_ctx)
{
    size_t i;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;

    test_ctx->p = test_ctx->cps;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, form);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    lxb_unicode_flush_count_set(uc, flush_count);

    for (i = 0; i < length; i += chunk) {
        status = lxb_unicode_normalize(uc, data + i,
                                       lexbor_min(chunk, length - i),
                                       callback_flushed, test_ctx, false);
        if (status != LXB_STATUS_OK) {
            goto done;
        }

        status = lxb_unicode_flush(uc, callback_flushed, test_ctx);
        if (status != LXB_STATUS_OK) {
            goto done;
        }
    }

    status = lxb_unicode_normalize_end(uc, callback_flushed, test_ctx);

done:

    (void) lxb_unicode_normalizer_destroy(uc, true);

    return status;
}

static lxb_status_t
normalize_flushed_cp(lxb_unicode_form_t form, const lxb_codepoint_t *cps,
                     size_t length, size_t chunk, size_t flush_count,
                     test_unicode_ctx_t *test_ctx)
{
    size_t i;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;

    test_ctx->p = test_ctx->cps;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, form);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    lxb_unicode_flush_count_set(uc, flush_count);

    for (i = 0; i < length; i += chunk) {
        status = lxb_unicode_normalize_cp(uc, cps + i,
                                          lexbor_min(chunk, length - i),
                                          callback_cp, test_ctx, false);
        if (status != LXB_STATUS_OK) {
            goto done;
        }

        status = lxb_unicode_flush_cp(uc, callback_cp, test_ctx);
        if (status != LXB_STATUS_OK) {
            goto done;
        }
    }

    status = lxb_unicode_normalize_cp_end(uc, callback_cp, test_ctx);

done:

    (void) lxb_unicode_normalizer_destroy(uc, true);

    return status;
}

TEST_BEGIN_ARGS(test_flush_calls, const lxb_char_t *data, size_t length,
                const size_t *counts, size_t counts_length)
{
    size_t f, i, c, len, cps_length;
    lxb_status_t status;
    const lxb_char_t *p, *end;
    test_unicode_ctx_t whole, flushed;
    lxb_codepoint_t cps[8192];

    static const lxb_unicode_form_t forms[] = {
        LXB_UNICODE_NFC, LXB_UNICODE_NFD, LXB_UNICODE_NFKC, LXB_UNICODE_NFKD
    };

    static const size_t chunks[] = {1, 2, 3, 7, 512};

    p = data;
    end = data + length;
    cps_length = 0;

    while (p < end) {
        cps[cps_length++] = lxb_encoding_decode_valid_utf_8_single(&p, end);
    }

    for (f = 0; f < sizeof(forms) / sizeof(forms[0]); f++) {
        status = normalize_flush(forms[f], data, length, SIZE_MAX, &whole);
        test_eq(status, LXB_STATUS_OK);

        len = whole.p - whole.cps;

        for (c = 0; c < counts_length; c++) {
            for (i = 0; i < sizeof(chunks) / sizeof(chunks[0]); i++) {
                TEST_PRINTLN("Form: %d; flush count: "LEXBOR_FORMAT_Z"; "
                             "chunk: "LEXBOR_FORMAT_Z, (int) forms[f],
                             counts[c], chunks[i]);

                status = normalize_flushed(forms[f], data, length, chunks[i],
                                           counts[c], &flushed);
                test_eq(status, LXB_STATUS_OK);

                test_eq_size((flushed.p - flushed.cps), len);
                test_eq(memcmp(flushed.cps, whole.cps,
                               len * sizeof(lxb_codepoint_t)), 0);

                status = normalize_flushed_cp(forms[f], cps, cps_length,
                                              chunks[i], counts[c], &flushed);
                test_eq(status, LXB_STATUS_OK);

                test_eq_size((flushed.p - flushed.cps), len);
                test_eq(memcmp(flushed.cps, whole.cps,
                               len * sizeof(lxb_codepoint_t)), 0);
            }
        }
    }
}
TEST_END

TEST_BEGIN(flush_mid_stream)
{
    size_t i;
    size_t counts[32];
    lxb_char_t source[2 + 4200 * 2 + 1];

    static const lxb_char_t mixed[] = "ᄀ각ẛ̣Å"
                                      "ﬁཱི̈́각éx";

    static const lxb_char_t leading[] = "̖́ạ̀é"
                                        "ୋ각x"
                                        "̖́";

    static const size_t long_counts[] = {0, 1, 2, 1023, 1024, 1025, SIZE_MAX};

    for (i = 0; i < sizeof(counts) / sizeof(counts[0]) - 1; i++) {
        counts[i] = i;
    }

    counts[i] = SIZE_MAX;

    TEST_CALL_ARGS(test_flush_calls, mixed, sizeof(mixed) - 1, counts,
                   sizeof(counts) / sizeof(counts[0]));
    TEST_CALL_ARGS(test_flush_calls, leading, sizeof(leading) - 1, counts,
                   sizeof(counts) / sizeof(counts[0]));

    memset(source, 'a', 1024);
    memcpy(source + 1024, "\xC3\xA9" "bcd", 5);

    TEST_CALL_ARGS(test_flush_calls, source, 1024 + 5, long_counts,
                   sizeof(long_counts) / sizeof(long_counts[0]));

    source[0] = 'a';
    source[1] = 'b';

    for (i = 0; i < 4200; i++) {
        memcpy(source + 2 + i * 2, "\xCC\x81", 2);
    }

    source[2 + 4200 * 2] = 'c';

    TEST_CALL_ARGS(test_flush_calls, source, sizeof(source), long_counts,
                   sizeof(long_counts) / sizeof(long_counts[0]));
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(chunks);
    TEST_ADD(edge);
    TEST_ADD(one_by_one);
    TEST_ADD(flush_compaction_hangul_tail);
    TEST_ADD(flush_compaction_mark_tail);
    TEST_ADD(flush_full_buffer);
    TEST_ADD(utf_8_split_sequences);
    TEST_ADD(utf_8_incomplete_end);
    TEST_ADD(utf_8_invalid_chunk_end);
    TEST_ADD(quick_check_incomplete_end);
    TEST_ADD(flush_decomposition);
    TEST_ADD(flush_mid_stream);

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
