/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <string.h>
#include <stdio.h>

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
    size_t read_size;
    lxb_status_t status;
    lxb_codepoint_t cp = 0x0000;
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

    status = lxb_encoding_decode_init_single(&decode, encoding);
    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to init decoder");
    }

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

        while (data < end) {
            cp = encoding->decode_single(&decode, &data, end);
            if (cp > LXB_ENCODING_DECODE_MAX_CODEPOINT) {
                if (cp == LXB_ENCODING_DECODE_CONTINUE) {
                    break;
                }

                printf("\\u%04X", LXB_ENCODING_REPLACEMENT_CODEPOINT);
                continue;
            }

            if (cp >= 0x00A0) {
                /* Code point is Unicode */
                printf("\\u%04X", cp);
            }
            else {
                /* Code point is ASCII */
                printf("\\x%02X", cp);
            }
        }
    }
    while (loop);

    if (cp == LXB_ENCODING_DECODE_CONTINUE) {
        printf("\\u%04X", LXB_ENCODING_REPLACEMENT_CODEPOINT);
    }

    return EXIT_SUCCESS;
}
