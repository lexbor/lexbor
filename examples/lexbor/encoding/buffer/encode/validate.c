/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <string.h>

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
    lxb_status_t status;
    lxb_encoding_encode_t encode;
    const lxb_codepoint_t *cps_ref, *cps_end;
    const lxb_encoding_data_t *encoding;

    /* Prepare buffer */
    lxb_char_t buffer[1024];

    /* Unicode code points for encoding */
    lxb_codepoint_t cps[] = {0x041F, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442,
                             0x002C,
                             0x110000, /* <-- bad code point */
                             0x0020, 0x043C, 0x0438, 0x0440, 0x0021, 0};

    cps_ref = cps;
    cps_end = cps_ref + (sizeof(cps) / sizeof(lxb_codepoint_t));

    /* Initialization */
    encoding = lxb_encoding_data(LXB_ENCODING_UTF_8);

    status = lxb_encoding_encode_init(&encode, encoding, buffer, sizeof(buffer));
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to initialization encoder");
    }

    status = lxb_encoding_encode_replace_set(&encode, LXB_ENCODING_REPLACEMENT_BYTES,
                                             LXB_ENCODING_REPLACEMENT_SIZE);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to set replacement bytes for encoder");
    }

    printf("Encode code points to UTF-8 byte string:\n");

    /* Encode */
    status = encoding->encode(&encode, &cps_ref, cps_end);
    if (status != LXB_STATUS_OK) {
        /* In this example, this cannot happen. */
    }

    /* Terminate string */
    buffer[ lxb_encoding_encode_buf_used(&encode) ] = 0x00;

    /* Print result */
    cps_ref = cps;

    for (; cps_ref < cps_end; cps_ref++) {
        printf("0x%04X", *cps_ref);
    }

    printf("\nResult: %s\n", (char *) buffer);

    return EXIT_SUCCESS;
}
