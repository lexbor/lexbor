/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/encoding/encoding.h>

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

static const lxb_char_t *
escaped_to_codepoint(const lxb_char_t *data, const lxb_char_t *end,
                     lxb_codepoint_t *cp, int8_t *state)
{
    if (*state != 0) {
        if (*state == 1) {
            goto check_format;
        }

        if (*state == 2) {
            goto check_bad_escape;
        }

        goto process;
    }

    if (*data++ != '\\') {
        goto failed;
    }

    if (data == end) {
        *state = 1;
        return data;
    }

check_format:

    if (*data != 'x' && *data != 'u') {
        goto failed;
    }

    if (++data == end) {
        *state = 2;
        return data;
    }

check_bad_escape:

    if (*data == '\\') {
        goto failed;
    }

    *cp = 0x0000;

process:

    while (data < end) {
        if (lexbor_str_res_map_hex[*data] == LEXBOR_STR_RES_SLIP) {
            if (*data != '\\') {
                if (*data == '\n') {
                    *state = 0;

                    return data + 1;
                }

                goto failed;
            }

            *state = 0;

            return data;
        }

        if (*cp > 0x10FFFF) {
            goto failed;
        }

        *cp <<= 4;
        *cp |= lexbor_str_res_map_hex[*data];

        data++;
    }

    *state = 3;

    return data;

failed:

    printf("Broken sequence of escaped code points\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, const char *argv[])
{
    int8_t state;
    int8_t len = 0;
    size_t read_size;
    lxb_status_t status;
    lxb_codepoint_t cp = 0x0000;
    lxb_encoding_encode_t encode;
    const lxb_encoding_data_t *encoding;

    /* Prepare buffer */
    char inbuf[4096];
    const lxb_char_t *data;
    const lxb_char_t *end;

    lxb_char_t outbuf[16];
    lxb_char_t *out;
    const lxb_char_t *out_end = outbuf + sizeof(outbuf);

    bool loop = true;

    if (argc != 2) {
        usage();
        exit(EXIT_SUCCESS);
    }

    state = 0;

    /* Determine encoding from first argument from command line */
    encoding = lxb_encoding_data_by_pre_name((const lxb_char_t *) argv[1],
                                             strlen(argv[1]));
    if (encoding == NULL) {
        FAILED(true, "Failed to get encoding from name: %s\n", argv[1]);
    }

    status = lxb_encoding_encode_init_single(&encode, encoding);
    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to init encoder");
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
            data = escaped_to_codepoint(data, end, &cp, &state);
            if (state != 0) {
                if (loop || state != 3) {
                    break;
                }

                state = 0;
            }

            out = outbuf;

            len = encoding->encode_single(&encode, &out, out_end, cp);
            if (len < LXB_ENCODING_ENCODE_OK) {
                if (len == LXB_ENCODING_ENCODE_SMALL_BUFFER) {
                    /* In this example, this cannot happen. */

                    FAILED(false, "Failed to converting code point to bytes");
                }

                
                if (encoding->encoding == LXB_ENCODING_UTF_8) {
                    printf("%s", LXB_ENCODING_REPLACEMENT_BYTES);
                    continue;
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

    if (state != 0) {
        FAILED(false, "Broken sequence of escaped code points\n");
    }

    if (len < LXB_ENCODING_ENCODE_OK) {
        if (encoding->encoding == LXB_ENCODING_UTF_8) {
            printf("%s", LXB_ENCODING_REPLACEMENT_BYTES);
        }
        else {
            printf("?");
        }
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
