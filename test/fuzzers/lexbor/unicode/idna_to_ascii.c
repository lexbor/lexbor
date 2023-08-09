/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/unicode/unicode.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t length)
{
    lxb_status_t status;
    lxb_unicode_idna_t idna;

    status = lxb_unicode_idna_init(&idna);
    if (status != LXB_STATUS_OK) {
        printf("Failed to init IDNA object.\n");
        return EXIT_FAILURE;
    }

    status = lxb_unicode_idna_to_ascii(&idna, data, length, callback, NULL, 0);
    if (status != LXB_STATUS_OK) {
        /* It is normal. Can be. */
    }

    (void) lxb_unicode_idna_destroy(&idna, false);

    return EXIT_SUCCESS;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    return LXB_STATUS_OK;
}
