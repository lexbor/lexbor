/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */


#include <lexbor/unicode/unicode.h>
#include <lexbor/encoding/encoding.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;

    /* ẛ̣ */
    lxb_char_t source[] = "\u1E9B\u0323";

    printf("Unicode Normalization Form for: 1E9B 0323 (\u1E9B\u0323):\n");

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, LXB_UNICODE_NFC);
    if (status != LXB_STATUS_OK) {
        printf("Failed to init unicode object.\n");
        return EXIT_FAILURE;
    }

    /* NFC */

    status = lxb_unicode_normalize(uc, source, sizeof(source) - 1,
                                   callback, "NFC", true);
    if (status != LXB_STATUS_OK) {
        printf("Failed to normalize NFC.\n");
        return EXIT_FAILURE;
    }

    /* NFD */

    (void) lxb_unicode_normalization_form_set(uc, LXB_UNICODE_NFD);
    status = lxb_unicode_normalize(uc, source, sizeof(source) - 1,
                                   callback, "NFD", true);
    if (status != LXB_STATUS_OK) {
        printf("Failed to normalize NFD.\n");
        return EXIT_FAILURE;
    }

    /* NFKC */

    (void) lxb_unicode_normalization_form_set(uc, LXB_UNICODE_NFKC);
    status = lxb_unicode_normalize(uc, source, sizeof(source) - 1,
                                   callback, "NFKC", true);
    if (status != LXB_STATUS_OK) {
        printf("Failed to normalize NFKC.\n");
        return EXIT_FAILURE;
    }

    /* NFKD */

    (void) lxb_unicode_normalization_form_set(uc, LXB_UNICODE_NFKD);
    status = lxb_unicode_normalize(uc, source, sizeof(source) - 1,
                                   callback, "NFKD", true);
    if (status != LXB_STATUS_OK) {
        printf("Failed to normalize NFKD.\n");
        return EXIT_FAILURE;
    }

    (void) lxb_unicode_normalizer_destroy(uc, true);

    return EXIT_SUCCESS;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lxb_codepoint_t cp;
    const lxb_char_t *p, *end;
    const char *name = ctx;

    p = data;
    end = data + len;

    printf("%s: ", name);

    while (p < end) {
        cp = lxb_encoding_decode_valid_utf_8_single(&p, end);

        printf("%04x ", cp);
    }

    printf("(%.*s)\n", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}
