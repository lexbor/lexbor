/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/unicode/unicode.h>
#include <lexbor/encoding/encoding.h>


typedef struct {
    lxb_char_t *data;
    size_t      length;
    size_t      size;
    size_t      calls;
}
test_idna_ctx_t;

typedef struct {
    lxb_codepoint_t *data;
    size_t          length;
    size_t          size;
    size_t          calls;
}
test_idna_cp_ctx_t;


/* Ten U+00E4 characters per label. */
static const lexbor_str_t unicode_label = lexbor_str(
    "\u00E4\u00E4\u00E4\u00E4\u00E4\u00E4\u00E4\u00E4\u00E4\u00E4");

static const lexbor_str_t ascii_label = lexbor_str("xn--4caaaaaaaaaa");


static void *
test_malloc(size_t size)
{
    void *data;

    /* Make a missing copy fail independently of recycled heap contents. */
    data = malloc(size);
    if (data != NULL) {
        memset(data, 0xA5, size);
    }

    return data;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    test_idna_ctx_t *buf = ctx;

    buf->calls++;

    if (len > buf->size - buf->length) {
        return LXB_STATUS_ERROR_SMALL_BUFFER;
    }

    memcpy(buf->data + buf->length, data, len);

    buf->length += len;

    return LXB_STATUS_OK;
}

static lxb_status_t
callback_cp(const lxb_codepoint_t *data, size_t len, void *ctx,
             lxb_status_t status)
{
    test_idna_cp_ctx_t *buf = ctx;

    if (status != LXB_STATUS_OK) {
        return status;
    }

    buf->calls++;

    if (len > buf->size - buf->length) {
        return LXB_STATUS_ERROR_SMALL_BUFFER;
    }

    memcpy(buf->data + buf->length, data, len * sizeof(lxb_codepoint_t));

    buf->length += len;

    return LXB_STATUS_OK;
}

static lxb_char_t *
test_domain(const lexbor_str_t *label, size_t labels, size_t *out_len)
{
    size_t i, o, total;
    lxb_char_t *domain;

    total = labels * (label->length + 1) - 1;

    domain = lexbor_malloc(total + 1);
    if (domain == NULL) {
        return NULL;
    }

    o = 0;

    for (i = 0; i < labels; i++) {
        if (i != 0) {
            domain[o++] = '.';
        }

        memcpy(domain + o, label->data, label->length);
        o += label->length;
    }

    domain[o] = 0x00;
    *out_len = o;

    return domain;
}

TEST_BEGIN_ARGS(test_convert_domain, const lexbor_str_t *input,
                const lexbor_str_t *output,
                size_t labels, bool to_ascii)
{
    size_t i, src_len, exp_len, cp_len;
    lxb_status_t status;
    lxb_char_t *source, *expected, *result;
    lxb_codepoint_t *cps;
    const lxb_char_t *p, *end;
    lxb_unicode_idna_t idna;
    test_idna_ctx_t buf;

    source = test_domain(input, labels, &src_len);
    test_ne(source, NULL);

    expected = test_domain(output, labels, &exp_len);
    test_ne(expected, NULL);

    result = lexbor_malloc(exp_len + 1);
    test_ne(result, NULL);

    cps = lexbor_malloc(src_len * sizeof(lxb_codepoint_t));
    test_ne(cps, NULL);

    p = source;
    end = source + src_len;
    cp_len = 0;

    while (p < end) {
        cps[cp_len++] = lxb_encoding_decode_valid_utf_8_single(&p, end);
    }

    buf.data = result;
    buf.size = exp_len;

    /* Exercise both UTF-8 and codepoint entry points with the same domain. */
    for (i = 0; i < 2; i++) {
        TEST_PRINTLN("Labels: "LEXBOR_FORMAT_Z"; to_ascii: %u; "
                     "codepoint_input: "LEXBOR_FORMAT_Z,
                     labels, (unsigned) to_ascii, i);

        buf.length = 0;
        buf.calls = 0;

        status = lxb_unicode_idna_init(&idna);
        test_eq(status, LXB_STATUS_OK);

        if (to_ascii) {
            if (i == 0) {
                status = lxb_unicode_idna_to_ascii(&idna, source, src_len,
                                                   callback, &buf, 0);
            }
            else {
                status = lxb_unicode_idna_to_ascii_cp(&idna, cps, cp_len,
                                                      callback, &buf, 0);
            }
        }
        else if (i == 0) {
            status = lxb_unicode_idna_to_unicode(&idna, source, src_len,
                                                 callback, &buf, 0);
        }
        else {
            status = lxb_unicode_idna_to_unicode_cp(&idna, cps, cp_len,
                                                    callback, &buf, 0);
        }

        test_eq(status, LXB_STATUS_OK);
        test_eq_size(buf.calls, 1);
        test_eq_str_n(result, buf.length, expected, exp_len);

        (void) lxb_unicode_idna_destroy(&idna, false);
    }

    lexbor_free(cps);
    lexbor_free(result);
    lexbor_free(expected);
    lexbor_free(source);
}
TEST_END

TEST_BEGIN_ARGS(test_process_domain, size_t prefix, bool mapped)
{
    size_t i, j, length;
    lxb_status_t status;
    lxb_char_t *source;
    lxb_codepoint_t *cps, *expected, *result;
    lxb_unicode_idna_t idna;
    test_idna_cp_ctx_t buf;

    /*
     * One ASCII label isolates the processing buffer: no byte-output buffer
     * or Punycode conversion is involved. U+FB03 maps to three codepoints,
     * "ffi", and can require growth before the old buffer is completely full.
     */

    length = prefix + (mapped ? 3 : 0);

    source = lexbor_malloc(length);
    test_ne(source, NULL);

    cps = lexbor_malloc((prefix + 1) * sizeof(lxb_codepoint_t));
    test_ne(cps, NULL);

    expected = lexbor_malloc(length * sizeof(lxb_codepoint_t));
    test_ne(expected, NULL);

    result = lexbor_malloc(length * sizeof(lxb_codepoint_t));
    test_ne(result, NULL);

    memset(source, 'a', prefix);

    for (i = 0; i < prefix; i++) {
        cps[i] = 'a';
        expected[i] = 'a';
    }

    if (mapped) {
        memcpy(source + prefix, "\xEF\xAC\x83", 3);
        cps[prefix] = 0xFB03;
        expected[prefix] = 'f';
        expected[prefix + 1] = 'f';
        expected[prefix + 2] = 'i';
    }

    buf.data = result;
    buf.size = length;

    for (i = 0; i < 2; i++) {
        TEST_PRINTLN("Prefix: "LEXBOR_FORMAT_Z"; mapped: %u; "
                     "codepoint_input: "LEXBOR_FORMAT_Z,
                     prefix, (unsigned) mapped, i);

        buf.length = 0;
        buf.calls = 0;

        status = lxb_unicode_idna_init(&idna);
        test_eq(status, LXB_STATUS_OK);

        if (i == 0) {
            status = lxb_unicode_idna_processing(&idna, source, length,
                                                 callback_cp, &buf, 0);
        }
        else {
            status = lxb_unicode_idna_processing_cp(&idna, cps,
                                                    prefix + (mapped ? 1 : 0),
                                                    callback_cp, &buf, 0);
        }

        test_eq(status, LXB_STATUS_OK);
        test_eq_size(buf.calls, 1);
        test_eq_size(buf.length, length);

        for (j = 0; j < length; j++) {
            test_eq(result[j], expected[j]);
        }

        (void) lxb_unicode_idna_destroy(&idna, false);
    }

    lexbor_free(result);
    lexbor_free(cps);
    lexbor_free(expected);
    lexbor_free(source);
}
TEST_END

TEST_BEGIN(processing_buffer_boundary)
{
    TEST_CALL_ARGS(test_process_domain, 4095, false);
    TEST_CALL_ARGS(test_process_domain, 4096, false);
    TEST_CALL_ARGS(test_process_domain, 4097, false);
}
TEST_END

TEST_BEGIN(processing_mapped_buffer)
{
    TEST_CALL_ARGS(test_process_domain, 4095, true);
}
TEST_END

TEST_BEGIN(to_ascii_output_buffer)
{
    /* 241 labels grow the byte buffer, while input stays below 4096 cps. */
    TEST_CALL_ARGS(test_convert_domain, &unicode_label, &ascii_label,
                   240, true);
    TEST_CALL_ARGS(test_convert_domain, &unicode_label, &ascii_label,
                   241, true);
    TEST_CALL_ARGS(test_convert_domain, &unicode_label, &ascii_label,
                   300, true);
}
TEST_END

TEST_BEGIN(to_unicode_output_buffer)
{
    /* Unicode input isolates byte-buffer growth on the 196th label. */
    TEST_CALL_ARGS(test_convert_domain, &unicode_label, &unicode_label,
                   195, false);
    TEST_CALL_ARGS(test_convert_domain, &unicode_label, &unicode_label,
                   196, false);
    TEST_CALL_ARGS(test_convert_domain, &unicode_label, &unicode_label,
                   300, false);
}
TEST_END

TEST_BEGIN(to_ascii_over_buffer)
{
    /* 4399 input codepoints and 6799 output bytes grow both buffers. */
    TEST_CALL_ARGS(test_convert_domain, &unicode_label, &ascii_label,
                   400, true);
}
TEST_END

TEST_BEGIN(to_unicode_over_buffer)
{
    /* 6799 input codepoints decode to 8399 UTF-8 output bytes. */
    TEST_CALL_ARGS(test_convert_domain, &ascii_label, &unicode_label,
                   400, false);
}
TEST_END

TEST_BEGIN(repeated_buffer_growth)
{
    /* Exercise heap realloc after the initial stack-to-heap copy. */
    TEST_CALL_ARGS(test_process_domain, 16386, false);
    TEST_CALL_ARGS(test_convert_domain, &unicode_label, &ascii_label,
                   1600, true);
    TEST_CALL_ARGS(test_convert_domain, &ascii_label, &unicode_label,
                   1600, false);
}
TEST_END

int
main(int argc, const char * argv[])
{
    lxb_status_t status;

    TEST_INIT();

    status = lexbor_memory_setup(test_malloc, realloc, calloc, free);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to set test allocator");
    }

    TEST_ADD(processing_buffer_boundary);
    TEST_ADD(processing_mapped_buffer);
    TEST_ADD(to_ascii_output_buffer);
    TEST_ADD(to_unicode_output_buffer);
    TEST_ADD(to_ascii_over_buffer);
    TEST_ADD(to_unicode_over_buffer);
    TEST_ADD(repeated_buffer_growth);

    TEST_RUN("lexbor/unicode/idna_large");

    (void) lexbor_memory_setup(malloc, realloc, calloc, free);

    TEST_RELEASE();
}
