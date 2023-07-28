/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/punycode/punycode.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


int
main(int argc, const char * argv[])
{
    bool loop;
    size_t size, nsize;
    lxb_status_t status;
    char inbuf[4096];
    lxb_char_t *buf, *end, *p, *tmp;

    buf = lexbor_malloc(sizeof(inbuf));
    if (buf == NULL) {
        printf("Failed memory allocation.\n");
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
                return EXIT_FAILURE;
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

    status = lxb_punycode_decode(buf, p - buf, callback, NULL);
    if (status != LXB_STATUS_OK) {
        printf("Failed decode.\n");
        goto failed;
    }

    printf("\n");

    lexbor_free(buf);

    return EXIT_SUCCESS;

failed:

    lexbor_free(buf);

    return EXIT_FAILURE;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}
