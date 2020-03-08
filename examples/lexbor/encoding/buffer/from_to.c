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
    size_t size;
    lxb_status_t status, encode_status, decode_status;
    lxb_encoding_encode_t encode;
    lxb_encoding_decode_t decode;
    const lxb_encoding_data_t *from, *to;

    bool loop = true;

    /* Encode */
    lxb_char_t outbuf[4096];

    /* Decode */
    lxb_codepoint_t cp[4096];
    const lxb_codepoint_t *cp_ref, *cp_end;

    /* Incoming from stdin */
    char inbuf[4096];
    const lxb_char_t *data, *end;

    if (argc != 3) {
        usage();
        exit(EXIT_SUCCESS);
    }

    /* Get encoding data for 'from' */
    from = lxb_encoding_data_by_pre_name((const lxb_char_t *) argv[1],
                                         strlen(argv[1]));
    if (from == NULL) {
        FAILED(true, "Failed to get encoding from name: %s\n", argv[1]);
    }

    /* Get encoding data for 'to' */
    to = lxb_encoding_data_by_pre_name((const lxb_char_t *) argv[2],
                                       strlen(argv[2]));
    if (to == NULL) {
        FAILED(true, "Failed to get encoding from name: %s\n", argv[2]);
    }

    /* Initialization decode */
    status = lxb_encoding_decode_init(&decode, from, cp,
                                      sizeof(cp) / sizeof(lxb_codepoint_t));
    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to initialization decoder");
    }

    status = lxb_encoding_decode_replace_set(&decode,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to set replacement code point for decoder");
    }

    /* Initialization encode */
    status = lxb_encoding_encode_init(&encode, to, outbuf, sizeof(outbuf));
    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to initialization encoder");
    }

    if (to->encoding == LXB_ENCODING_UTF_8) {
        status = lxb_encoding_encode_replace_set(&encode,
                 LXB_ENCODING_REPLACEMENT_BYTES, LXB_ENCODING_REPLACEMENT_SIZE);
    }
    else {
        status = lxb_encoding_encode_replace_set(&encode, (lxb_char_t *) "?", 1);
    }

    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to set replacement bytes for encoder");
    }

    do {
        /* Read standart input */
        size = fread(inbuf, 1, sizeof(inbuf), stdin);
        if (size != sizeof(inbuf)) {
            if (feof(stdin)) {
                loop = false;
            }
            else {
                FAILED(false, "Failed to read stdin");
            }
        }

        /* Decode incoming data */
        data = (const lxb_char_t *) inbuf;
        end = data + size;

        do {
            /* Decode */
            decode_status = from->decode(&decode, &data, end);

            cp_ref = cp;
            cp_end = cp + lxb_encoding_decode_buf_used(&decode);

            do {
                encode_status = to->encode(&encode, &cp_ref, cp_end);
                if (encode_status == LXB_STATUS_ERROR) {
                    cp_ref++;
                    encode_status = LXB_STATUS_SMALL_BUFFER;
                }

                size = lxb_encoding_encode_buf_used(&encode);

                /* The printf function cannot print \x00, it can be in UTF-16 */
                if (fwrite(outbuf, 1, size, stdout) != size) {
                    FAILED(false, "Failed to write data to stdout");
                }

                lxb_encoding_encode_buf_used_set(&encode, 0);
            }
            while (encode_status == LXB_STATUS_SMALL_BUFFER);

            lxb_encoding_decode_buf_used_set(&decode, 0);
        }
        while (decode_status == LXB_STATUS_SMALL_BUFFER);
    }
    while (loop);

    /* End of file */
    /* In this moment encoder and decoder out buffer is empty */

    /*
     * First: finish decoding.
     * If there was not enough data to create the code point,
     * then the replacement character will put to the buffer.
     */
    (void) lxb_encoding_decode_finish(&decode);

    if (lxb_encoding_decode_buf_used(&decode)) {
        cp_ref = cp;
        cp_end = cp + lxb_encoding_decode_buf_used(&decode);

        (void) to->encode(&encode, &cp_ref, cp_end);
        size = lxb_encoding_encode_buf_used(&encode);

        if (fwrite(outbuf, 1, size, stdout) != size) {
            FAILED(false, "Failed to write data to stdout");
        }
    }

    /*
     * Second: finish encoding.
     * The truth is that this is only necessary for one encoding: iso-2022-jp.
     */
    (void) lxb_encoding_encode_finish(&encode);
    size = lxb_encoding_encode_buf_used(&encode);

    if (size != 0) {
        if (fwrite(outbuf, 1, size, stdout) != size) {
            FAILED(false, "Failed to write data to stdout");
        }
    }

    return EXIT_SUCCESS;
}
