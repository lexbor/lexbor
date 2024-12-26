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


#include "unicode_normalization_test_res.h"


static lxb_status_t
test_unicode_nf(lxb_unicode_form_t form);

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);

static lxb_char_t *
to_ansi(const lxb_codepoint_t *cps, lxb_char_t *buf, const lxb_char_t *end);

static void
printf_cps(const lxb_codepoint_t *cps);

static void
printf_ansi(const lxb_char_t *data, const lxb_char_t *end);


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
    size_t i;
    const char *name;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;
    const lxb_codepoint_t *cps;
    const lxb_char_t *p_source, *p_nf;
    const lxb_unicode_test_t *test = lxb_unicode_test_entries;

    lxb_char_t source[64];
    const lxb_char_t *source_end = source + sizeof(source);

    lxb_char_t nf[64];
    const lxb_char_t *nf_end = nf + sizeof(nf);

    lxb_char_t buffer[64];
    lexbor_str_t str = {.data = buffer, .length = 0};

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

        p_source = to_ansi(test->source, source, source_end);
        p_nf = to_ansi(cps, nf, nf_end);

        printf("Test #"LEXBOR_FORMAT_Z"\n", ++i);

        str.length = 0;

        status = lxb_unicode_normalize(uc, source, p_source - source,
                                       callback, &str, true);
        if (status != LXB_STATUS_OK) {
            printf("Failed to convert to NFC\n");
            goto failed;
        }

        if (str.length != p_nf - nf
            || memcmp(buffer, nf, p_nf - nf) != 0)
        {
            printf("Length not match: in buffer = "LEXBOR_FORMAT_Z"; "
                   "in %s test = %ld\n", str.length, name, p_nf - nf);

            printf("Sorc codepoints: ");
            printf_cps(test->source);
            printf("\n");

            printf("Want codepoints: ");
            printf_cps(cps);
            printf("\n");

            printf("Have codepoints: ");
            printf_ansi(buffer, buffer + str.length);
            printf("\n");

            printf("%.*s != %.*s\n",
                   (int) (p_nf - nf), nf,
                   (int) str.length, (char *) buffer);

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
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lexbor_str_t *str = ctx;

    memcpy(&str->data[str->length], data, len);

    str->length += len;
    str->data[str->length] = '\0';

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

static void
printf_cps(const lxb_codepoint_t *cps)
{
    while (*cps != LXB_ENCODING_MAX_CODEPOINT) {
        printf("%x ", *cps);
        cps += 1;
    }
}

static void
printf_ansi(const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_codepoint_t cp;

    while (data < end) {
        cp = lxb_encoding_decode_valid_utf_8_single(&data, end);
        printf("%x ", cp);
    }
}
