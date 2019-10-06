/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <string.h>

#include <lexbor/encoding/encoding.h>


int
main(int argc, const char *argv[])
{
    int8_t len;
    lxb_encoding_encode_t ctx = {0};
    const lxb_encoding_data_t *encoding;
    const lxb_char_t *pos;

    /* Prepare buffer */
    lxb_char_t buffer[1024];
    lxb_char_t *data = buffer;
    const lxb_char_t *end = data + sizeof(buffer);

    /* Unicode code points for encoding */
    lxb_codepoint_t cps[] = {0x041F, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442,
                             0x002C, 0x0020, 0x043C, 0x0438, 0x0440, 0x0021, 0};

    encoding = lxb_encoding_data(LXB_ENCODING_UTF_8);

    printf("Encode code points to UTF-8 byte string:\n");

    for (size_t i = 0; cps[i] != 0; i++) {
        pos = data;

        len = encoding->encode(&ctx, &data, end, cps[i]);
        if (len < LXB_ENCODING_ENCODE_OK) {

            /* In this example, this cannot happen. */
            continue;
        }

        printf("0x%04X: %.*s\n", cps[i], len, pos);
    }

    /* Terminate string */
    *data = 0x00;

    printf("\nResult: %s\n", (char *) buffer);

    return EXIT_SUCCESS;
}
