/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/unicode/unicode.h>


typedef struct {
    lxb_codepoint_t *data;
    size_t          length;
    size_t          size;
}
test_idna_ctx_t;

typedef struct {
    lxb_char_t *data;
    size_t     length;
}
test_idna_str_t;


static const lxb_codepoint_t decoded_cps[] = {
    'm', 'a', 0x00F1, 'a', 'n', 'a', '.', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.'
};

static const lxb_codepoint_t hyphen_cps[] = {'-', 0x00F1, '.'};


static lxb_status_t
callback(const lxb_codepoint_t *part, size_t len, void *ctx,
         lxb_status_t status)
{
    test_idna_ctx_t *buf = ctx;

    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (len + 1 > buf->size - buf->length) {
        return LXB_STATUS_ERROR_SMALL_BUFFER;
    }

    memcpy(buf->data + buf->length, part, len * sizeof(lxb_codepoint_t));

    buf->length += len;
    buf->data[buf->length++] = '.';

    return LXB_STATUS_OK;
}

static lxb_status_t
callback_null(const lxb_codepoint_t *part, size_t len, void *ctx,
              lxb_status_t status)
{
    if (ctx != NULL) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    return status;
}

static lxb_status_t
callback_str(const lxb_char_t *data, size_t len, void *ctx)
{
    test_idna_str_t *str = ctx;

    str->data = lexbor_malloc(len + 1);
    if (str->data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    memcpy(str->data, data, len);

    str->data[len] = 0x00;
    str->length = len;

    return LXB_STATUS_OK;
}

static lxb_codepoint_t *
test_to_cps(const lexbor_str_t *str, size_t *out_len)
{
    size_t i;
    lxb_codepoint_t *cps;

    cps = lexbor_malloc(str->length * sizeof(lxb_codepoint_t));
    if (cps == NULL) {
        return NULL;
    }

    for (i = 0; i < str->length; i++) {
        cps[i] = str->data[i];
    }

    *out_len = str->length;

    return cps;
}

TEST_BEGIN_ARGS(test_process, const lexbor_str_t *input,
                lxb_unicode_idna_flag_t flags, lxb_status_t need_status,
                const lxb_codepoint_t *need, size_t need_len)
{
    size_t i, j, cps_len;
    lxb_status_t status;
    lxb_codepoint_t *cps;
    lxb_codepoint_t result[64];
    lxb_unicode_idna_t idna;
    test_idna_ctx_t *buf;

    cps = test_to_cps(input, &cps_len);
    test_ne(cps, NULL);

    for (i = 0; i < 2; i++) {
        TEST_PRINTLN("Input: %.*s; flags: %u; codepoint_input: "LEXBOR_FORMAT_Z,
                     (int) input->length, (const char *) input->data,
                     (unsigned) flags, i);

        /* The exact size lets ASan see any access past the caller context. */
        buf = lexbor_malloc(sizeof(test_idna_ctx_t));
        test_ne(buf, NULL);

        buf->data = result;
        buf->length = 0;
        buf->size = sizeof(result) / sizeof(lxb_codepoint_t);

        status = lxb_unicode_idna_init(&idna);
        test_eq(status, LXB_STATUS_OK);

        if (i == 0) {
            status = lxb_unicode_idna_processing(&idna, input->data,
                                                 input->length, callback,
                                                 buf, flags);
        }
        else {
            status = lxb_unicode_idna_processing_cp(&idna, cps, cps_len,
                                                    callback, buf, flags);
        }

        test_eq(status, need_status);

        if (need_status == LXB_STATUS_OK) {
            test_eq_size(buf->length, need_len);

            for (j = 0; j < need_len; j++) {
                test_eq(result[j], need[j]);
            }
        }

        (void) lxb_unicode_idna_destroy(&idna, false);

        lexbor_free(buf);
    }

    lexbor_free(cps);
}
TEST_END

TEST_BEGIN_ARGS(test_process_null, const lexbor_str_t *input)
{
    size_t i, cps_len;
    lxb_status_t status;
    lxb_codepoint_t *cps;
    lxb_unicode_idna_t idna;

    cps = test_to_cps(input, &cps_len);
    test_ne(cps, NULL);

    for (i = 0; i < 2; i++) {
        TEST_PRINTLN("Input: %.*s; codepoint_input: "LEXBOR_FORMAT_Z,
                     (int) input->length, (const char *) input->data, i);

        status = lxb_unicode_idna_init(&idna);
        test_eq(status, LXB_STATUS_OK);

        if (i == 0) {
            status = lxb_unicode_idna_processing(&idna, input->data,
                                                 input->length, callback_null,
                                                 NULL, 0);
        }
        else {
            status = lxb_unicode_idna_processing_cp(&idna, cps, cps_len,
                                                    callback_null, NULL, 0);
        }

        test_eq(status, LXB_STATUS_OK);

        (void) lxb_unicode_idna_destroy(&idna, false);
    }

    lexbor_free(cps);
}
TEST_END

TEST_BEGIN_ARGS(test_convert, const lexbor_str_t *input,
                lxb_unicode_idna_flag_t flags, bool to_ascii,
                lxb_status_t need_status, const lexbor_str_t *need)
{
    lxb_status_t status;
    lxb_unicode_idna_t idna;
    test_idna_str_t str;

    TEST_PRINTLN("Input: %.*s; flags: %u; to_ascii: %u",
                 (int) input->length, (const char *) input->data,
                 (unsigned) flags, (unsigned) to_ascii);

    str.data = NULL;
    str.length = 0;

    status = lxb_unicode_idna_init(&idna);
    test_eq(status, LXB_STATUS_OK);

    if (to_ascii) {
        status = lxb_unicode_idna_to_ascii(&idna, input->data, input->length,
                                           callback_str, &str, flags);
    }
    else {
        status = lxb_unicode_idna_to_unicode(&idna, input->data, input->length,
                                             callback_str, &str, flags);
    }

    test_eq(status, need_status);

    if (need_status == LXB_STATUS_OK) {
        test_eq_str_n(str.data, str.length, need->data, need->length);
    }

    (void) lxb_unicode_idna_destroy(&idna, false);

    if (str.data != NULL) {
        lexbor_free(str.data);
    }
}
TEST_END

TEST_BEGIN(processing_punycode_context)
{
    lexbor_str_t ace = lexbor_str("xn--maana-pta.example");

    TEST_CALL_ARGS(test_process, &ace, LXB_UNICODE_IDNA_FLAG_UNDEF,
                   LXB_STATUS_OK, decoded_cps,
                   sizeof(decoded_cps) / sizeof(lxb_codepoint_t));
}
TEST_END

TEST_BEGIN(processing_punycode_null_context)
{
    lexbor_str_t ace = lexbor_str("xn--maana-pta.example");
    lexbor_str_t hyphen = lexbor_str("xn----rga");

    TEST_CALL_ARGS(test_process_null, &ace);
    TEST_CALL_ARGS(test_process_null, &hyphen);
}
TEST_END

TEST_BEGIN(processing_punycode_flags)
{
    lexbor_str_t hyphen = lexbor_str("xn----rga");

    TEST_CALL_ARGS(test_process, &hyphen, LXB_UNICODE_IDNA_FLAG_UNDEF,
                   LXB_STATUS_OK, hyphen_cps,
                   sizeof(hyphen_cps) / sizeof(lxb_codepoint_t));

    TEST_CALL_ARGS(test_process, &hyphen, LXB_UNICODE_IDNA_FLAG_CHECK_HYPHENS,
                   LXB_STATUS_ERROR_UNEXPECTED_RESULT, NULL, 0);
}
TEST_END

TEST_BEGIN(convert_punycode_flags)
{
    lexbor_str_t hyphen = lexbor_str("xn----rga");
    lexbor_str_t hyphen_unicode = lexbor_str("-\xC3\xB1");

    TEST_CALL_ARGS(test_convert, &hyphen, LXB_UNICODE_IDNA_FLAG_UNDEF,
                   true, LXB_STATUS_OK, &hyphen);

    TEST_CALL_ARGS(test_convert, &hyphen, LXB_UNICODE_IDNA_FLAG_CHECK_HYPHENS,
                   true, LXB_STATUS_ERROR_UNEXPECTED_RESULT, NULL);

    TEST_CALL_ARGS(test_convert, &hyphen, LXB_UNICODE_IDNA_FLAG_UNDEF,
                   false, LXB_STATUS_OK, &hyphen_unicode);

    TEST_CALL_ARGS(test_convert, &hyphen, LXB_UNICODE_IDNA_FLAG_CHECK_HYPHENS,
                   false, LXB_STATUS_ERROR_UNEXPECTED_RESULT, NULL);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(processing_punycode_context);
    TEST_ADD(processing_punycode_null_context);
    TEST_ADD(processing_punycode_flags);
    TEST_ADD(convert_punycode_flags);

    TEST_RUN("lexbor/unicode/idna_processing");
    TEST_RELEASE();
}
