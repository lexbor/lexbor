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
                                                                               \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    while (0)


int
main(int argc, const char *argv[])
{
    size_t buf_length;
    lxb_status_t status;
    lxb_codepoint_t cp[32];
    lxb_encoding_decode_t decode;
    const lxb_encoding_data_t *encoding;

    /* Prepare buffer */
    const lxb_char_t *data = (const lxb_char_t *) "Привет, мир!";
    const lxb_char_t *end = data + strlen((char *) data);

    /* Initialization */
    encoding = lxb_encoding_data(LXB_ENCODING_UTF_8);

    status  = lxb_encoding_decode_init(&decode, encoding, cp,
                                       sizeof(cp) / sizeof(lxb_codepoint_t));
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to initialization decoder");
    }

    printf("Decode UTF-8 string \"%s\" to code points:\n", (char *) data);

    /* Decode */
    status = encoding->decode(&decode, &data, end);
    if (status != LXB_STATUS_OK) {
        /* In this example, this cannot happen. */
    }

    /* Print result */
    buf_length = lxb_encoding_decode_buf_used(&decode);

    for (size_t i = 0; i < buf_length; i++) {
        printf("0x%04X\n", cp[i]);
    }

    return EXIT_SUCCESS;
}
