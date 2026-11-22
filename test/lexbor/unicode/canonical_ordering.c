/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/unicode/unicode.h>
#include <lexbor/encoding/encoding.h>


typedef struct {
    lxb_codepoint_t *data;
    size_t          length;
    size_t          size;
}
test_cps_t;

typedef struct {
    lxb_codepoint_t cp;
    uint8_t         ccc;
}
test_mark_t;


static const test_mark_t test_marks[] = {
    {0x0334, 1}, {0x093C, 7}, {0x05B0, 10}, {0x05C1, 24}, {0x0670, 35},
    {0x0E48, 107}, {0x0F71, 129}, {0x0F72, 130}, {0x0327, 202},
    {0x1DCE, 214}, {0x031B, 216}, {0x0316, 220}, {0x0317, 220},
    {0x0301, 230}, {0x0300, 230}, {0x0315, 232}, {0x035C, 233},
    {0x035D, 234}, {0x0345, 240}
};

static const lxb_unicode_form_t test_forms[] = {
    LXB_UNICODE_NFC, LXB_UNICODE_NFD, LXB_UNICODE_NFKC, LXB_UNICODE_NFKD
};


static lxb_status_t
test_cps_push(test_cps_t *cps, lxb_codepoint_t cp)
{
    size_t size;
    lxb_codepoint_t *data;

    if (cps->length == cps->size) {
        size = (cps->size == 0) ? 1024 : cps->size * 2;

        data = lexbor_realloc(cps->data, size * sizeof(lxb_codepoint_t));
        if (data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        cps->data = data;
        cps->size = size;
    }

    cps->data[cps->length++] = cp;

    return LXB_STATUS_OK;
}

static lxb_status_t
test_cps_push_cps(test_cps_t *cps, const test_cps_t *from)
{
    size_t i;
    lxb_status_t status;

    for (i = 0; i < from->length; i++) {
        status = test_cps_push(cps, from->data[i]);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

static void
test_cps_destroy(test_cps_t *cps)
{
    cps->data = lexbor_free(cps->data);
    cps->length = 0;
    cps->size = 0;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lxb_status_t status;
    lxb_codepoint_t cp;
    const lxb_char_t *end;

    end = data + len;

    while (data < end) {
        cp = lxb_encoding_decode_valid_utf_8_single(&data, end);

        status = test_cps_push(ctx, cp);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
callback_cp(const lxb_codepoint_t *cps, size_t len, void *ctx)
{
    size_t i;
    lxb_status_t status;

    for (i = 0; i < len; i++) {
        status = test_cps_push(ctx, cps[i]);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
test_marks_build(size_t *indexes, size_t length, uint32_t seed,
                 test_cps_t *marks, test_cps_t *sorted)
{
    size_t i;
    unsigned ccc;
    lxb_status_t status;

    static const size_t count = sizeof(test_marks) / sizeof(test_mark_t);

    for (i = 0; i < length; i++) {
        seed = seed * 1103515245 + 12345;
        indexes[i] = (seed >> 16) % count;

        status = test_cps_push(marks, test_marks[indexes[i]].cp);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    for (ccc = 0; ccc < 256; ccc++) {
        for (i = 0; i < length; i++) {
            if (test_marks[indexes[i]].ccc == ccc) {
                status = test_cps_push(sorted, test_marks[indexes[i]].cp);
                if (status != LXB_STATUS_OK) {
                    return status;
                }
            }
        }
    }

    return LXB_STATUS_OK;
}

TEST_BEGIN_ARGS(test_forms_check, const test_cps_t *source,
                const test_cps_t *composed, const test_cps_t *decomposed)
{
    size_t f, i, chunk, length;
    lxb_char_t *utf8, *p;
    lxb_status_t status;
    test_cps_t result;
    const test_cps_t *need;
    lxb_unicode_normalizer_t *uc;

    utf8 = lexbor_malloc(source->length * 4);
    test_ne(utf8, NULL);

    p = utf8;

    for (i = 0; i < source->length; i++) {
        (void) lxb_encoding_encode_utf_8_single(NULL, &p,
                                                utf8 + source->length * 4,
                                                source->data[i]);
    }

    length = p - utf8;

    result.data = NULL;
    result.length = 0;
    result.size = 0;

    for (f = 0; f < sizeof(test_forms) / sizeof(lxb_unicode_form_t); f++) {
        need = (test_forms[f] == LXB_UNICODE_NFC
                || test_forms[f] == LXB_UNICODE_NFKC) ? composed : decomposed;

        for (i = 0; i < 4; i++) {
            TEST_PRINTLN("Form: %d; source length: "LEXBOR_FORMAT_Z"; "
                         "mode: "LEXBOR_FORMAT_Z, (int) test_forms[f],
                         source->length, i);

            result.length = 0;

            uc = lxb_unicode_normalizer_create();
            status = lxb_unicode_normalizer_init(uc, test_forms[f]);
            test_eq(status, LXB_STATUS_OK);

            switch (i) {
                case 0:
                    status = lxb_unicode_normalize(uc, utf8, length, callback,
                                                   &result, true);
                    break;

                case 1:
                    for (chunk = 0; chunk < length; chunk += 7) {
                        status = lxb_unicode_normalize(uc, utf8 + chunk,
                                                lexbor_min(7, length - chunk),
                                                callback, &result, false);
                        test_eq(status, LXB_STATUS_OK);
                    }

                    status = lxb_unicode_normalize_end(uc, callback, &result);
                    break;

                case 2:
                    status = lxb_unicode_normalize_cp(uc, source->data,
                                                      source->length,
                                                      callback_cp, &result,
                                                      true);
                    break;

                default:
                    for (chunk = 0; chunk < source->length; chunk += 7) {
                        status = lxb_unicode_normalize_cp(uc,
                                        source->data + chunk,
                                        lexbor_min(7, source->length - chunk),
                                        callback_cp, &result, false);
                        test_eq(status, LXB_STATUS_OK);
                    }

                    status = lxb_unicode_normalize_cp_end(uc, callback_cp,
                                                          &result);
                    break;
            }

            test_eq(status, LXB_STATUS_OK);
            test_eq_size(result.length, need->length);
            test_eq(memcmp(result.data, need->data,
                           need->length * sizeof(lxb_codepoint_t)), 0);

            (void) lxb_unicode_normalizer_destroy(uc, true);
        }
    }

    test_cps_destroy(&result);
    lexbor_free(utf8);
}
TEST_END

TEST_BEGIN_ARGS(test_runs, const test_cps_t *prefix, const test_cps_t *nfc,
                const test_cps_t *nfd, size_t length, uint32_t seed)
{
    size_t *indexes;
    lxb_status_t status;
    test_cps_t marks = {0}, sorted = {0}, source = {0};
    test_cps_t composed = {0}, decomposed = {0};

    indexes = lexbor_malloc((length + 1) * sizeof(size_t));
    test_ne(indexes, NULL);

    status = test_marks_build(indexes, length, seed, &marks, &sorted);
    test_eq(status, LXB_STATUS_OK);

    test_eq(test_cps_push_cps(&source, prefix), LXB_STATUS_OK);
    test_eq(test_cps_push_cps(&source, &marks), LXB_STATUS_OK);
    test_eq(test_cps_push(&source, 'x'), LXB_STATUS_OK);

    test_eq(test_cps_push_cps(&composed, nfc), LXB_STATUS_OK);
    test_eq(test_cps_push_cps(&composed, &sorted), LXB_STATUS_OK);
    test_eq(test_cps_push(&composed, 'x'), LXB_STATUS_OK);

    test_eq(test_cps_push_cps(&decomposed, nfd), LXB_STATUS_OK);
    test_eq(test_cps_push_cps(&decomposed, &sorted), LXB_STATUS_OK);
    test_eq(test_cps_push(&decomposed, 'x'), LXB_STATUS_OK);

    TEST_CALL_ARGS(test_forms_check, &source, &composed, &decomposed);

    test_cps_destroy(&decomposed);
    test_cps_destroy(&composed);
    test_cps_destroy(&source);
    test_cps_destroy(&sorted);
    test_cps_destroy(&marks);
    lexbor_free(indexes);
}
TEST_END

TEST_BEGIN(long_run)
{
    size_t i, n;
    test_cps_t source = {0}, composed = {0}, decomposed = {0};

    n = 50000;

    test_eq(test_cps_push(&source, 'a'), LXB_STATUS_OK);
    test_eq(test_cps_push(&composed, 0x00E1), LXB_STATUS_OK);
    test_eq(test_cps_push(&decomposed, 'a'), LXB_STATUS_OK);

    for (i = 0; i < n; i++) {
        test_eq(test_cps_push(&source, 0x0301), LXB_STATUS_OK);
        test_eq(test_cps_push(&source, 0x0316), LXB_STATUS_OK);

        test_eq(test_cps_push(&composed, 0x0316), LXB_STATUS_OK);
        test_eq(test_cps_push(&decomposed, 0x0316), LXB_STATUS_OK);
    }

    for (i = 0; i < n; i++) {
        if (i != 0) {
            test_eq(test_cps_push(&composed, 0x0301), LXB_STATUS_OK);
        }

        test_eq(test_cps_push(&decomposed, 0x0301), LXB_STATUS_OK);
    }

    TEST_CALL_ARGS(test_forms_check, &source, &composed, &decomposed);

    test_eq(test_cps_push(&source, 'x'), LXB_STATUS_OK);
    test_eq(test_cps_push(&composed, 'x'), LXB_STATUS_OK);
    test_eq(test_cps_push(&decomposed, 'x'), LXB_STATUS_OK);

    TEST_CALL_ARGS(test_forms_check, &source, &composed, &decomposed);

    test_cps_destroy(&decomposed);
    test_cps_destroy(&composed);
    test_cps_destroy(&source);
}
TEST_END

TEST_BEGIN(mixed_runs)
{
    size_t length;
    test_cps_t prefix = {0};

    test_eq(test_cps_push(&prefix, 'x'), LXB_STATUS_OK);

    for (length = 0; length <= 80; length++) {
        TEST_CALL_ARGS(test_runs, &prefix, &prefix, &prefix, length,
                       (uint32_t) length);
        TEST_CALL_ARGS(test_runs, &prefix, &prefix, &prefix, length,
                       (uint32_t) length + 1000);
    }

    TEST_CALL_ARGS(test_runs, &prefix, &prefix, &prefix, 1000, 1);
    TEST_CALL_ARGS(test_runs, &prefix, &prefix, &prefix, 20000, 2);

    test_cps_destroy(&prefix);
}
TEST_END

TEST_BEGIN(hangul_runs)
{
    test_cps_t prefix = {0}, nfc = {0};

    test_eq(test_cps_push(&prefix, 0x1100), LXB_STATUS_OK);
    test_eq(test_cps_push(&prefix, 0x1161), LXB_STATUS_OK);
    test_eq(test_cps_push(&nfc, 0xAC00), LXB_STATUS_OK);

    TEST_CALL_ARGS(test_runs, &prefix, &nfc, &prefix, 3, 3);
    TEST_CALL_ARGS(test_runs, &prefix, &nfc, &prefix, 40, 4);
    TEST_CALL_ARGS(test_runs, &prefix, &nfc, &prefix, 20000, 5);

    test_cps_destroy(&nfc);
    test_cps_destroy(&prefix);
}
TEST_END

TEST_BEGIN(leading_runs)
{
    test_cps_t prefix = {0};

    TEST_CALL_ARGS(test_runs, &prefix, &prefix, &prefix, 2, 6);
    TEST_CALL_ARGS(test_runs, &prefix, &prefix, &prefix, 40, 7);
    TEST_CALL_ARGS(test_runs, &prefix, &prefix, &prefix, 20000, 8);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(long_run);
    TEST_ADD(mixed_runs);
    TEST_ADD(hangul_runs);
    TEST_ADD(leading_runs);

    TEST_RUN("lexbor/unicode/canonical_ordering");
    TEST_RELEASE();
}
