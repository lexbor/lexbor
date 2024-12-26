/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/unicode/unicode.h>


typedef struct {
    const lxb_char_t *source;
    const lxb_char_t *ascii;
    size_t           status;
}
lxb_unicode_idna_test_t;


#include "unicode_idna_test_res.h"


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lexbor_str_t *str = ctx;

    memcpy(str->data, data, len);

    str->length = len;

    return LXB_STATUS_OK;
}

int
main(int argc, const char *argv[])
{
    size_t n, source_len, ascii_len;
    lxb_status_t status;
    lexbor_str_t str;
    lxb_unicode_idna_t idna;
    const lxb_unicode_idna_test_t *p;
    lxb_char_t data[4096 * 6];

    status = lxb_unicode_idna_init(&idna);
    if (status != LXB_STATUS_OK) {
        printf("Failed to init IDNA object\n");
        return EXIT_FAILURE;
    }

    str.data = data;

    n = 1;
    p = lxb_unicode_idna_test_entries;

    while (p->source != NULL) {
        printf("Test #"LEXBOR_FORMAT_Z": ", n++);

        source_len = strlen((const char *) p->source);
        ascii_len = strlen((const char *) p->ascii);

        str.length = 0;

        status = lxb_unicode_idna_to_ascii(&idna, p->source,source_len,
                                           callback, &str,
                                           LXB_UNICODE_IDNA_FLAG_USE_STD3ASCII_RULES);
        if (status != LXB_STATUS_OK) {
            if (p->status != 0) {
                printf("expected bad status\n");
                goto next;
            }

            return EXIT_FAILURE;
        }

        if (str.length != ascii_len
            || memcmp(str.data, p->ascii, ascii_len) != 0)
        {
            printf("failed\n");

            printf("Result not match:\n");
            printf("Need: %.*s\n", (int) ascii_len, (const char *) p->ascii);
            printf("Have: %.*s\n", (int) str.length, (const char *) str.data);

            return EXIT_FAILURE;
        }

        printf("ok\n");

    next:

        p += 1;
    }

    (void) lxb_unicode_idna_destroy(&idna, false);

    return EXIT_SUCCESS;
}
