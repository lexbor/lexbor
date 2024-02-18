/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/encoding/encoding.h>


#define FAILED(...)                                                            \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    while (0)


int
main(int argc, const char *argv[])
{
    lxb_codepoint_t cp;
    lxb_status_t status;
    lxb_encoding_decode_t decode;
    const lxb_encoding_data_t *encoding;
    const lxb_char_t *pos;

    /* Prepare buffer */
    const lxb_char_t *data = (const lxb_char_t *) "Привет, мир!";
    const lxb_char_t *end = data + strlen((char *) data);

    encoding = lxb_encoding_data(LXB_ENCODING_UTF_8);

    status = lxb_encoding_decode_init_single(&decode, encoding);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to init decoder");
    }

    printf("Decode UTF-8 string \"%s\" to code points:\n", (char *) data);

    while (data < end) {
        pos = data;

        cp = encoding->decode_single(&decode, &data, end);
        if (cp > LXB_ENCODING_DECODE_MAX_CODEPOINT) {

            /* In this example, this cannot happen. */
            continue;
        }

        printf("%.*s: 0x%04X\n", (int) (data - pos), pos, cp);
    }

    return EXIT_SUCCESS;
}
