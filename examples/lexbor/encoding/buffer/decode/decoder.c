/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/encoding/encoding.h>


#define FAILED(with_usage, ...)                                                \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
                                                                               \
        if (with_usage) {                                                      \
            usage();                                                           \
        }                                                                      \
                                                                               \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    while (0)


static void
usage(void)
{
    printf("Usage: decoder <encoding name>\n\n");
    printf("Available encodings:\n");
    printf("big5, euc-jp, euc-kr, gbk, ibm866, iso-2022-jp, iso-8859-10, iso-8859-13,\n");
    printf("iso-8859-14, iso-8859-15, iso-8859-16, iso-8859-2, iso-8859-3, iso-8859-4,\n");
    printf("iso-8859-5, iso-8859-6, iso-8859-7, iso-8859-8, iso-8859-8-i, koi8-r, koi8-u,\n");
    printf("shift_jis, utf-16be, utf-16le, utf-8, gb18030, macintosh, replacement,\n");
    printf("windows-1250, windows-1251, windows-1252, windows-1253, windows-1254,\n");
    printf("windows-1255, windows-1256, windows-1257, windows-1258, windows-874,\n");
    printf("x-mac-cyrillic, x-user-defined.\n");
}

int
main(int argc, const char *argv[])
{
    size_t read_size, buf_length;
    lxb_status_t status;
    lxb_codepoint_t cp[4096];
    lxb_encoding_decode_t decode;
    const lxb_encoding_data_t *encoding;

    /* Prepare buffer */
    char inbuf[4096];
    const lxb_char_t *data;
    const lxb_char_t *end;

    bool loop = true;

    if (argc != 2) {
        usage();
        exit(EXIT_SUCCESS);
    }

    /* Determine encoding from first argument from command line */
    encoding = lxb_encoding_data_by_pre_name((const lxb_char_t *) argv[1],
                                             strlen(argv[1]));
    if (encoding == NULL) {
        FAILED(true, "Failed to get encoding from name: %s\n\n", argv[1]);
    }

    /* Initialization */
    status = lxb_encoding_decode_init(&decode, encoding, cp,
                                      sizeof(cp) / sizeof(lxb_codepoint_t));
    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to initialization decoder");
    }

    status = lxb_encoding_decode_replace_set(&decode, LXB_ENCODING_REPLACEMENT_BUFFER,
                                             LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to set replacement code points for decoder");
    }

    /* Decode */
    do {
        /* Read standart input */
        read_size = fread(inbuf, 1, sizeof(inbuf), stdin);
        if (read_size != sizeof(inbuf)) {
            if (feof(stdin)) {
                loop = false;
            }
            else {
                FAILED(false, "Failed to read stdin");
            }
        }

        /* Decode incoming data */
        data = (const lxb_char_t *) inbuf;
        end = data + read_size;

        do {
            status = encoding->decode(&decode, &data, end);

            buf_length = lxb_encoding_decode_buf_used(&decode);

            for (size_t i = 0; i < buf_length; i++) {
                if (cp[i] >= 0x00A0) {
                    /* Code point is Unicode */
                    printf("\\u%04X", cp[i]);
                }
                else {
                    /* Code point is ASCII */
                    printf("\\x%02X", cp[i]);
                }
            }

            lxb_encoding_decode_buf_used_set(&decode, 0);
        }
        while (status == LXB_STATUS_SMALL_BUFFER);
    }
    while (loop);

    /* No point checking status. */
    (void) lxb_encoding_decode_finish(&decode);

    /*
     * We need to check the out buffer after calling the finish function.
     * If there was not enough data to form a code point, then the finish
     * function will be added the replacement character to the out buffer.
     */
    buf_length = lxb_encoding_decode_buf_used(&decode);

    if (buf_length != 0) {
        for (size_t i = 0; i < buf_length; i++) {
            if (cp[i] >= 0x00A0) {
                printf("\\u%04X", cp[i]);
            }
            else {
                printf("\\x%02X", cp[i]);
            }
        }
    }

    return EXIT_SUCCESS;
}
