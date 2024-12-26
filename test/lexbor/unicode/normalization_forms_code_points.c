/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/unicode/unicode.h>
#include <lexbor/encoding/encoding.h>


typedef struct {
    const lxb_codepoint_t *source;
    const lxb_codepoint_t *nfc;
    const lxb_codepoint_t *nfd;
    const lxb_codepoint_t *nfkc;
    const lxb_codepoint_t *nfkd;
}
lxb_unicode_test_t;

typedef struct {
    lxb_codepoint_t *data;
    size_t          length;
}
lxb_unicode_test_cp_t;


#include "unicode_normalization_test_res.h"


static lxb_status_t
test_unicode_nf(lxb_unicode_form_t form);

static lxb_status_t
callback(const lxb_codepoint_t *data, size_t len, void *ctx);

static lxb_char_t *
to_ansi(const lxb_codepoint_t *cps, lxb_char_t *buf, const lxb_char_t *end);

static size_t
cps_length(const lxb_codepoint_t *cps);

static void
printf_cps(const lxb_codepoint_t *cps);


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_unicode_form_t nfs[4] = {
        LXB_UNICODE_NFC, LXB_UNICODE_NFD, LXB_UNICODE_NFKC, LXB_UNICODE_NFKD
    };

    for (size_t i = 0; i < 4; i++) {
        status = test_unicode_nf(nfs[i]);
        if (status != LXB_STATUS_OK) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static lxb_status_t
test_unicode_nf(lxb_unicode_form_t form)
{
    size_t i, src_len, nf_len;
    const char *name;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;
    const lxb_codepoint_t *cps;
    const lxb_unicode_test_t *test = lxb_unicode_test_entries;

    lxb_char_t source[64];
    const lxb_char_t *source_end = source + sizeof(source);

    lxb_char_t nf[64];
    const lxb_char_t *nf_end = nf + sizeof(nf);

    lxb_codepoint_t buffer[64];
    lxb_unicode_test_cp_t res = {.data = buffer, .length = 0};

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, form);
    if (status != LXB_STATUS_OK) {
        printf("Failed to init unicode object.\n");
        goto failed;
    }

    switch (form) {
        case LXB_UNICODE_NFC:  name = "NFC";  break;
        case LXB_UNICODE_NFD:  name = "NFD";  break;
        case LXB_UNICODE_NFKC: name = "NFKC"; break;
        case LXB_UNICODE_NFKD: name = "NFKD"; break;
        default: goto failed;
    }

    printf("Begin %s tests:\n", name);

    i = 0;

    do {
        switch (form) {
            case LXB_UNICODE_NFC:  cps = test->nfc;  break;
            case LXB_UNICODE_NFD:  cps = test->nfd;  break;
            case LXB_UNICODE_NFKC: cps = test->nfkc; break;
            case LXB_UNICODE_NFKD: cps = test->nfkd; break;
            default: goto failed;
        }

        src_len = cps_length(test->source);
        nf_len = cps_length(cps);

        printf("Test #"LEXBOR_FORMAT_Z"\n", ++i);

        res.length = 0;

        status = lxb_unicode_normalize_cp(uc, test->source, src_len,
                                          callback, &res, true);
        if (status != LXB_STATUS_OK) {
            printf("Failed to convert to NFC\n");
            goto failed;
        }

        if (res.length != nf_len
            || memcmp(buffer, res.data, nf_len * sizeof(lxb_codepoint_t)) != 0)
        {
            printf("Length not match: in buffer = "LEXBOR_FORMAT_Z"; "
                   "in %s test = %ld\n", res.length, name, nf_len);

            printf("Sorc codepoints: ");
            printf_cps(test->source);
            printf("\n");

            printf("Want codepoints: ");
            printf_cps(cps);
            printf("\n");

            printf("Have codepoints: ");
            printf_cps(res.data);
            printf("\n");

            nf_end = to_ansi(cps, nf, nf_end);
            source_end = to_ansi(res.data, source, source_end);

            printf("%.*s != %.*s\n",
                   (int) nf_len, (const char *) nf,
                   (int) (source_end - source), (const char *) source);

            goto failed;
        }

        test += 1;
    }
    while (test->source != NULL);

    printf("Total %s tests: "LEXBOR_FORMAT_Z"\n", name, i);

    lxb_unicode_normalizer_destroy(uc, true);

    return LXB_STATUS_OK;

failed:

    lxb_unicode_normalizer_destroy(uc, true);

    return LXB_STATUS_ERROR;
}

static lxb_status_t
callback(const lxb_codepoint_t *cps, size_t len, void *ctx)
{
    lxb_unicode_test_cp_t *res = ctx;

    memcpy(&res->data[res->length], cps, len * sizeof(lxb_codepoint_t));

    res->length += len;
    res->data[res->length] = LXB_ENCODING_MAX_CODEPOINT;

    return LXB_STATUS_OK;
}

static lxb_char_t *
to_ansi(const lxb_codepoint_t *cps, lxb_char_t *buf, const lxb_char_t *end)
{
    while (*cps != LXB_ENCODING_MAX_CODEPOINT) {
        (void) lxb_encoding_encode_utf_8_single(NULL, &buf, end, *cps);
        cps += 1;
    }

    return buf;
}

static size_t
cps_length(const lxb_codepoint_t *cps)
{
    const lxb_codepoint_t *begin = cps;

    while (*cps != LXB_ENCODING_MAX_CODEPOINT) {
        cps += 1;
    }

    return cps - begin;
}

static void
printf_cps(const lxb_codepoint_t *cps)
{
    while (*cps != LXB_ENCODING_MAX_CODEPOINT) {
        printf("%x ", *cps);
        cps += 1;
    }
}
