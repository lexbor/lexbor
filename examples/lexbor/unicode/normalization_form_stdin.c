/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/unicode/unicode.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


int
main(int argc, const char * argv[])
{
    bool loop;
    size_t size;
    lxb_status_t status;
    lxb_unicode_form_t form;
    lxb_unicode_normalizer_t *uc;
    char inbuf[4096];

    if (argc < 2) {
        goto usage;
    }

    if (strlen(argv[1]) == 3) {
        if (memcmp(argv[1], "NFC", 3) == 0) {
            form = LXB_UNICODE_NFC;
        }
        else if (memcmp(argv[1], "NFD", 3) == 0) {
            form = LXB_UNICODE_NFD;
        }
        else {
            goto usage;
        }
    }
    else if (strlen(argv[1]) == 4) {
        if (memcmp(argv[1], "NFKC", 4) == 0) {
            form = LXB_UNICODE_NFKC;
        }
        else if (memcmp(argv[1], "NFKD", 4) == 0) {
            form = LXB_UNICODE_NFKD;
        }
        else {
            goto usage;
        }
    }
    else {
        goto usage;
    }

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, form);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    loop = true;

    do {
        size = fread(inbuf, 1, sizeof(inbuf), stdin);
        if (size != sizeof(inbuf)) {
            if (feof(stdin)) {
                loop = false;
            }
            else {
                return EXIT_FAILURE;
            }
        }

        status = lxb_unicode_normalize(uc, (const lxb_char_t *) inbuf, size,
                                       callback, NULL, !loop);
        if (status != LXB_STATUS_OK) {
            return EXIT_FAILURE;
        }
    }
    while (loop);

    (void) lxb_unicode_normalizer_destroy(uc, true);

    return EXIT_SUCCESS;

usage:

    printf("Usage: ./normalization_form_stdin <form>\n");
    printf("\t<form> one of: NFC, NFD, NFKC, NFKD\n");

    return EXIT_SUCCESS;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}
