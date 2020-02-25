/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <string.h>
#include <stdio.h>

#include <lexbor/encoding/encoding.h>
#include <lexbor/encoding/encode.h>

#define LEXBOR_STR_RES_MAP_HEX
#include <lexbor/core/str_res.h>


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
    printf("Usage: encoder <encoding name>\n\n");
    printf("Available encodings:\n");
    printf("big5, euc-jp, euc-kr, gbk, ibm866, iso-2022-jp, iso-8859-10, iso-8859-13,\n");
    printf("iso-8859-14, iso-8859-15, iso-8859-16, iso-8859-2, iso-8859-3, iso-8859-4,\n");
    printf("iso-8859-5, iso-8859-6, iso-8859-7, iso-8859-8, iso-8859-8-i, koi8-r, koi8-u,\n");
    printf("shift_jis, utf-16be, utf-16le, utf-8, gb18030, macintosh, replacement,\n");
    printf("windows-1250, windows-1251, windows-1252, windows-1253, windows-1254,\n");
    printf("windows-1255, windows-1256, windows-1257, windows-1258, windows-874,\n");
    printf("x-mac-cyrillic, x-user-defined.\n");
}

static const lxb_codepoint_t *
escaped_to_codepoint(const lxb_char_t *data, const lxb_char_t *end,
                     lxb_codepoint_t *cp, int8_t *state, lxb_codepoint_t *rep,
                     bool is_last)
{
    if (*state != 0) {
        if (*state == 1) {
            goto check_format;
        }

        if (*state == 2) {
            goto check_bad_escape;
        }

        *cp = *rep;
        *rep = 0x0000;

        goto process;
    }

    while (data < end) {
        if (*data++ != '\\') {
            goto failed;
        }

        if (data == end) {
            *state = 1;
            return cp;
        }

    check_format:

        if (*data != 'x' && *data != 'u') {
            goto failed;
        }

        if (++data == end) {
            *state = 2;
            return cp;
        }

    check_bad_escape:

        if (*data == '\\') {
            goto failed;
        }

        *cp = 0x0000;

    process:

        *state = 3;

        while (data < end) {
            if (lexbor_str_res_map_hex[*data] == LEXBOR_STR_RES_SLIP) {
                if (*data != '\\') {
                    if (*data == '\n') {
                        *state = 0;

                        cp++;
                        data++;
                        break;
                    }

                    goto failed;
                }

                *state = 0;

                cp++;
                break;
            }

            if (*cp > 0x10FFFF) {
                goto failed;
            }

            *cp <<= 4;
            *cp |= lexbor_str_res_map_hex[*data];

            data++;
        }
    }

    if (is_last) {
        if (*state == 3) {
            return cp + 1;
        }

        return cp;
    }

    if (*state == 3) {
        *rep = *cp;
    }

    return cp;

failed:

    printf("Broken sequence of escaped code points\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, const char *argv[])
{
    int8_t state;
    size_t read_size;
    lxb_status_t status;
    lxb_encoding_encode_t encode;
    const lxb_encoding_data_t *encoding;

    bool loop = true;

    /* Prepare buffer */
    char inbuf[4096];
    const lxb_char_t *data, *end;

    lxb_codepoint_t cp[4096];
    lxb_codepoint_t cp_rep = 0x0000;
    const lxb_codepoint_t *cp_ref, *cp_end;

    lxb_char_t outbuf[4096];

    if (argc != 2) {
        usage();
        exit(EXIT_SUCCESS);
    }

    state = 0;

    /* Initialization */

    /* Determine encoding from first argument from command line */
    encoding = lxb_encoding_data_by_pre_name((const lxb_char_t *) argv[1],
                                             strlen(argv[1]));
    if (encoding == NULL) {
        FAILED(true, "Failed to get encoding from name: %s\n", argv[1]);
    }

    status = lxb_encoding_encode_init(&encode, encoding, outbuf, sizeof(outbuf));
    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to initialization encoder");
    }

    if (encoding->encoding == LXB_ENCODING_UTF_8) {
        status = lxb_encoding_encode_replace_set(&encode,
                 LXB_ENCODING_REPLACEMENT_BYTES, LXB_ENCODING_REPLACEMENT_SIZE);
    }
    else {
        status = lxb_encoding_encode_replace_set(&encode, (lxb_char_t *) "?", 1);
    }

    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to set replacement bytes for encoder");
    }

    /* Encode */
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

        /* Encode incoming data */
        data = (const lxb_char_t *) inbuf;
        end = data + read_size;

        cp_end = escaped_to_codepoint(data, end, cp, &state, &cp_rep,
                                      loop == false);
        if (state != 0) {
            if (loop == false && state == 3) {
                state = 0;
            }
        }

        cp_ref = cp;

        do {
            status = encoding->encode(&encode, &cp_ref, cp_end);

            read_size = lxb_encoding_encode_buf_used(&encode);

            /* The printf function cannot print \x00, it can be in UTF-16 */
            if (fwrite(outbuf, 1, read_size, stdout) != read_size) {
                FAILED(false, "Failed to write data to stdout");
            }

            lxb_encoding_encode_buf_used_set(&encode, 0);
        }
        while (status == LXB_STATUS_SMALL_BUFFER);
    }
    while (loop);

    if (state != 0) {
        FAILED(false, "Broken sequence of escaped code points\n");
    }

    /* End of file */
    /* In this moment encoder out buffer is empty */

    (void) lxb_encoding_encode_finish(&encode);

    read_size = lxb_encoding_encode_buf_used(&encode);

    if (read_size != 0) {
        if (fwrite(outbuf, 1, read_size, stdout) != read_size) {
            FAILED(false, "Failed to write data to stdout");
        }
    }

    return EXIT_SUCCESS;
}
