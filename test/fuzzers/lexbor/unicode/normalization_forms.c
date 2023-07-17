/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */


#include <lexbor/unicode/unicode.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


#ifndef LEXBOR_HAVE_FUZZER
int
main(int argc, const char * argv[])
{
    bool loop;
    size_t size;
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;
    char inbuf[4096];

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, LXB_UNICODE_NFC);
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
}
#endif

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t length)
{
    lxb_status_t status;
    lxb_unicode_normalizer_t *uc;

    uc = lxb_unicode_normalizer_create();
    status = lxb_unicode_normalizer_init(uc, LXB_UNICODE_NFC);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    status = lxb_unicode_normalize(uc, (const lxb_char_t *) data, length,
                                   callback, NULL, true);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    (void) lxb_unicode_normalizer_destroy(uc, true);

    return EXIT_SUCCESS;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    return LXB_STATUS_OK;
}
