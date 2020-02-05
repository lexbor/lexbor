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
    printf("Usage: from_to <from> <to>\n\n");
    printf("Available encodings for 'from' and 'to':\n");
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
    int8_t len;
    size_t read_size;
    lxb_status_t status;
    lxb_codepoint_t cp;
    lxb_encoding_encode_t encode;
    lxb_encoding_decode_t decode;
    const lxb_encoding_data_t *from, *to;

    bool loop = true;

    /* Encode */
    lxb_char_t outbuf[16];
    lxb_char_t *out;
    const lxb_char_t *out_end = outbuf + sizeof(outbuf);

    /* Decode */
    const lxb_char_t *data;
    const lxb_char_t *end;

    /* Incoming from stdin */
    char inbuf[4096];

    if (argc != 3) {
        usage();
        exit(EXIT_SUCCESS);
    }

    /* Get encoding data for 'from' */
    from = lxb_encoding_data_by_pre_name((const lxb_char_t *) argv[1],
                                         strlen(argv[1]));
    if (from == NULL) {
        FAILED(true, "Failed to get encoding from name: %s", argv[1]);
    }

    /* Get encoding data for 'to' */
    to = lxb_encoding_data_by_pre_name((const lxb_char_t *) argv[2],
                                       strlen(argv[2]));
    if (to == NULL) {
        FAILED(true, "Failed to get encoding from name: %s", argv[2]);
    }

    status = lxb_encoding_encode_init_single(&encode, to);
    if (status != LXB_STATUS_OK) {
        FAILED(true, "Failed to init encoder");
    }

    status = lxb_encoding_decode_init_single(&decode, from);
    if (status != LXB_STATUS_OK) {
        FAILED(true, "Failed to init decoder");
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
            /* Decode */
            cp = from->decode_single(&decode, &data, end);
            if (cp > LXB_ENCODING_DECODE_MAX_CODEPOINT) {
                if (cp == LXB_ENCODING_DECODE_CONTINUE && loop) {
                    break;
                }

                cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
            }

            /* Encode */
            out = outbuf;

            len = to->encode_single(&encode, &out, out_end, cp);
            if (len < LXB_ENCODING_ENCODE_OK) {
                if (len == LXB_ENCODING_ENCODE_SMALL_BUFFER) {
                    /* In this example, this cannot happen. */
                    FAILED(false, "Failed to converting code point to bytes");
                }

                printf("?");
                continue;
            }

            /* The printf function cannot print \x00, it can be in UTF-16 */
            if (fwrite(outbuf, 1, len, stdout) != len) {
                FAILED(false, "Failed to write data to stdout");
            }
        }
    }
    while (loop);

    status = lxb_encoding_decode_finish_single(&decode);
    if (status != LXB_STATUS_OK) {
        printf("?");
    }

    out = outbuf;

    len = lxb_encoding_encode_finish_single(&encode, &out, out_end);
    if (len != 0) {
        if (fwrite(outbuf, 1, len, stdout) != len) {
            FAILED(false, "Failed to write data to stdout");
        }
    }

    return EXIT_SUCCESS;
}
