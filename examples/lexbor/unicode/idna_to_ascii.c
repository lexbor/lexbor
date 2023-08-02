/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/unicode/unicode.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


int
main(int argc, const char *argv[])
{
    bool loop;
    size_t size, nsize;
    lxb_status_t status;
    lxb_unicode_idna_t idna;
    lxb_char_t *buf, *end, *p, *tmp;
    char inbuf[4096];

    status = lxb_unicode_idna_init(&idna);
    if (status != LXB_STATUS_OK) {
        printf("Failed to init IDNA object.\n");
        return EXIT_FAILURE;
    }

    buf = lexbor_malloc(sizeof(inbuf));
    if (buf == NULL) {
        printf("Failed memory allocation.\n");

        lxb_unicode_idna_destroy(&idna, false);

        return EXIT_FAILURE;
    }

    p = buf;
    end = buf + sizeof(inbuf);

    loop = true;

    do {
        size = fread(inbuf, 1, sizeof(inbuf), stdin);
        if (size != sizeof(inbuf)) {
            if (feof(stdin)) {
                loop = false;
            }
            else {
                printf("Failed read stdin.\n");
                goto failed;
            }
        }

        if (p + size > end) {
            nsize = (end - buf) * 3;

            tmp = lexbor_realloc(buf, nsize);
            if (tmp == NULL) {
                printf("Failed memory reallocation.\n");
                goto failed;
            }

            p = tmp + (p - buf);
            buf = tmp;
            end = tmp + nsize;
        }

        memcpy(p, inbuf, size);

        p += size;
    }
    while (loop);

    if (p - buf > 0) {
        if (p[-1] == '\n') {
            p -= 1;
        }
    }

    if (p - buf > 0) {
        if (p[-1] == '\r') {
            p -= 1;
        }
    }

    status = lxb_unicode_idna_to_ascii(&idna, buf, p - buf, callback, NULL, 0);
    if (status != LXB_STATUS_OK) {
        printf("Failed convert to ASCII.\n");
        goto failed;
    }

    printf("\n");

    lexbor_free(buf);
    lxb_unicode_idna_destroy(&idna, false);

    return EXIT_SUCCESS;

failed:

    lexbor_free(buf);
    lxb_unicode_idna_destroy(&idna, false);

    return EXIT_FAILURE;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}
