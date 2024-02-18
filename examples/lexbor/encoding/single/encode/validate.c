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
    int8_t len;
    lxb_status_t status;
    lxb_encoding_encode_t encode;
    const lxb_encoding_data_t *encoding;
    const lxb_char_t *pos;

    /* Prepare buffer */
    lxb_char_t buffer[1024];
    lxb_char_t *data = buffer;
    const lxb_char_t *end = data + sizeof(buffer);

    /* Unicode code points for encoding */
    lxb_codepoint_t cps[] = {0x041F, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442,
                             0x002C, 
                             0x110000, /* <-- bad code point */
                             0x0020, 0x043C, 0x0438, 0x0440, 0x0021, 0};

    encoding = lxb_encoding_data(LXB_ENCODING_UTF_8);

    status = lxb_encoding_encode_init_single(&encode, encoding);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to init encoder");
    }

    printf("Validate and encode code points to UTF-8 byte string:\n");

    for (size_t i = 0; cps[i] != 0; i++) {
        pos = data;

        len = encoding->encode_single(&encode, &data, end, cps[i]);

        if (len < LXB_ENCODING_ENCODE_OK) {
            if (len == LXB_ENCODING_ENCODE_SMALL_BUFFER) {
                /* 
                 * Need more buffer in which we will write.
                 * In this example, this cannot happen.
                 */

                break;
            }

            printf("Bad code point: 0x%04X; Replaced to: %s (0x%04X)\n",
                   cps[i], LXB_ENCODING_REPLACEMENT_BYTES,
                   LXB_ENCODING_REPLACEMENT_CODEPOINT);

            /* Append Replacement character to data */
            memcpy(data, LXB_ENCODING_REPLACEMENT_BYTES,
                   LXB_ENCODING_REPLACEMENT_SIZE);

            data += LXB_ENCODING_REPLACEMENT_SIZE;

            continue;
        }

        printf("0x%04X: %.*s\n", cps[i], len, pos);
    }

    /* Terminate string */
    *data = 0x00;

    printf("\nResult: %s\n", (char *) buffer);

    return EXIT_SUCCESS;
}
