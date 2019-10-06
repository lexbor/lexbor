/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/encoding/encoding.h"
#include "lexbor/encoding/multi.h"
#include "lexbor/encoding/range.h"


#define LEXBOR_STR_RES_MAP_HEX
#define LEXBOR_STR_RES_MAP_NUM
#include "lexbor/core/str_res.h"


#define to_update_buffer(data)                                                 \
    buf = (lxb_char_t *) data;                                                 \
    end = buf + strlen((const char *) data)

#define to_update_buffer_size(data, size)                                      \
    buf = (lxb_char_t *) data;                                                 \
    end = buf + size

#define to_cps(count, ...) (lxb_codepoint_t [count]) {__VA_ARGS__}

#define test_buffer(func, count, ...)                                          \
    do {                                                                       \
        size = func(enc_data, buf, end, cps_buffer,                            \
             cps_buffer + (sizeof(cps_buffer) / sizeof(lxb_codepoint_t)));     \
        if (size != count) {                                                   \
            test_call_error();                                                 \
        }                                                                      \
                                                                               \
        test_eq(memcmp(to_cps(count, __VA_ARGS__),                             \
                       cps_buffer, sizeof(lxb_codepoint_t) * count), 0);       \
    }                                                                          \
    while (0)


typedef struct {
    lxb_char_t      data[16];
    size_t          size;
    lxb_codepoint_t cp;
}
lxb_test_entry_t;

typedef lxb_status_t
(*lxb_test_encoding_process_file_f)(const lxb_test_entry_t *entry, void *ctx,
                                    size_t line);


static lxb_status_t
test_encoding_process_file(const char *filename,
                           lxb_test_encoding_process_file_f fun_process,
                           void *ctx, size_t *line);


void
failed_and_exit(size_t line)
{
    printf("Failed to parse file; line: "LEXBOR_FORMAT_Z"\n", line);

    exit(EXIT_FAILURE);
}

static lxb_codepoint_t
test_decode_chunks(const lxb_encoding_data_t *enc_data,
                   lxb_char_t *buf, lxb_char_t *end)
{
    lxb_codepoint_t cp = 0x00;
    lxb_encoding_decode_t ctx = {0};

    lxb_char_t ch;
    const lxb_char_t *ch_ref, *ch_end;

    ch_ref = &ch;
    ch_end = ch_ref + 1;

    /* Imitate the work with chunks */
    while (buf < end) {
        ch = *buf++;

        cp = enc_data->decode(&ctx, &ch_ref, ch_end);
        if (cp > LXB_ENCODING_DECODE_MAX_CODEPOINT
            && cp != LXB_ENCODING_DECODE_CONTINUE)
        {
            return LXB_ENCODING_DECODE_ERROR;
        }

        ch_ref--;
    }

    return cp;
}

static lxb_codepoint_t
test_decode_full(const lxb_encoding_data_t *enc_data,
                 const lxb_char_t *buf, const lxb_char_t *end)
{
    lxb_codepoint_t cp = 0x00;
    lxb_encoding_decode_t ctx = {0};

    while (buf < end) {
        cp = enc_data->decode(&ctx, &buf, end);
        if (cp > LXB_ENCODING_DECODE_MAX_CODEPOINT) {
            return LXB_ENCODING_DECODE_ERROR;
        }
    }

    return cp;
}

static size_t
test_decode_buffer_chunks(const lxb_encoding_data_t *enc_data,
                          const lxb_char_t *buf, const lxb_char_t *end,
                          lxb_codepoint_t *cps, lxb_codepoint_t *cps_end)
{
    lxb_char_t ch;
    lxb_codepoint_t cp, *begin;
    const lxb_char_t *ch_ref, *ch_end;
    lxb_encoding_decode_t ctx = {0};

    ch_ref = &ch;
    ch_end = ch_ref + 1;
    begin = cps;

    /* Imitate the work with chunks */
    while (buf < end) {
        ch = *buf;

        cp = enc_data->decode(&ctx, &ch_ref, ch_end);

        if (ch_ref == ch_end) {
            ch_ref--;
            buf++;
        }

        if (cp > LXB_ENCODING_DECODE_MAX_CODEPOINT) {
            /* Need to save last code point == CONTINUE */
            if (cp == LXB_ENCODING_DECODE_CONTINUE) {
                if (buf < end) {
                    continue;
                }
            }
            else {
                cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
            }
        }

        *cps++ = cp;
        if (cps >= cps_end) {
            return 0;
        }
    }

    return cps - begin;
}

static size_t
test_decode_buffer_full(const lxb_encoding_data_t *enc_data,
                        const lxb_char_t *buf, const lxb_char_t *end,
                        lxb_codepoint_t *cps, lxb_codepoint_t *cps_end)
{
    lxb_codepoint_t cp, *begin;
    lxb_encoding_decode_t ctx = {0};

    begin = cps;

    while (buf < end) {
        cp = enc_data->decode(&ctx, &buf, end);

        if (cp > LXB_ENCODING_DECODE_MAX_CODEPOINT) {
            if (cp == LXB_ENCODING_DECODE_CONTINUE) {
                if (buf < end) {
                    continue;
                }
            }
            else {
                cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
            }
        }

        *cps++ = cp;
        if (cps >= cps_end) {
            return 0;
        }
    }

    return cps - begin;
}

lxb_inline int
skip_comment_line(FILE *fc, size_t *line)
{
    int ch;

    while ((ch = fgetc(fc)) != EOF) {
        if (ch == '\n') {
            (*line)++;
            return LXB_STATUS_OK;
        }
    }

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
skip_whitespace_line(FILE *fc, size_t *line)
{
    int ch;

    while ((ch = fgetc(fc)) != EOF) {
        if (ch == '\n') {
            (*line)++;
            return LXB_STATUS_OK;
        }

        if (ch != ' ' && ch != '\t') {
            return LXB_STATUS_ERROR;
        }
    }

    return ch;
}

lxb_inline size_t
read_hex(FILE *fc, size_t line)
{
    int ch;
    size_t num = 0;

    do {
        ch = fgetc(fc);
        if (ch == EOF) {
            goto seek_done;
        }

        if (lexbor_str_res_map_hex[(unsigned) ch] == LEXBOR_STR_RES_SLIP) {
            if (ch == ' ' || ch == '\\' || ch == '\n') {
                goto seek_done;
            }

            return LXB_ENCODING_DECODE_ERROR;
        }

        if (num <= 0x10FFFF) {
            num <<= 4;
            num |= lexbor_str_res_map_hex[ (unsigned) ch ];
        }
    }
    while (1);

    return num;

seek_done:

    fseek(fc , -1, SEEK_CUR);

    return num;
}

lxb_inline lxb_status_t
read_data(FILE *fc, size_t *line, lxb_test_entry_t *entry)
{
    int ch;
    size_t num;
    lxb_char_t *data = entry->data;

    while ((ch = fgetc(fc))) {
        if (ch != '\\') {
            fseek(fc , -1, SEEK_CUR);
            break;
        }

        ch = fgetc(fc);
        if (ch != 'x' || ch == EOF) {
            return LXB_STATUS_ERROR;
        }

        num = read_hex(fc, *line);
        if (num == LXB_ENCODING_DECODE_ERROR) {
            return LXB_STATUS_ERROR;
        }

        *data++ = (lxb_char_t) num;

        entry->size++;
    }

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
read_codepoint(FILE *fc, size_t *line, lxb_test_entry_t *entry)
{
    int ch = fgetc(fc);

    if (ch != '0') {
        return LXB_STATUS_ERROR;
    }

    ch = fgetc(fc);
    if (ch != 'x' || ch == EOF) {
        return LXB_STATUS_ERROR;
    }

    entry->cp = (lxb_codepoint_t) read_hex(fc, *line);
    if (entry->cp == LXB_ENCODING_DECODE_ERROR) {
        entry->cp = 0x00;
        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
test_encoding_process_file(const char *filename,
                           lxb_test_encoding_process_file_f fun_process,
                           void *ctx, size_t *line)
{
    int ch;
    lxb_status_t status;
    lxb_test_entry_t entry;

    *line = 0;

    FILE *fc = fopen(filename, "r");
    if (fc == NULL) {
        printf("Failed to opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    while ((ch = fgetc(fc)) != EOF) {
        switch (ch) {
            case '#':
                status = skip_comment_line(fc, line);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                break;

            case ' ':
            case '\t':
                status = skip_whitespace_line(fc, line);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                break;

            case '\n':
                (*line)++;
                break;

            case '\\':
                fseek(fc , -1, SEEK_CUR);
                memset(&entry, 0 , sizeof(lxb_test_entry_t));

                status = read_data(fc, line, &entry);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                ch = fgetc(fc);
                if (ch != ' ' || ch == EOF) {
                    return LXB_STATUS_ERROR;
                }

                status = read_codepoint(fc, line, &entry);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                status = fun_process(&entry, ctx, *line);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                break;

            default:
                return LXB_STATUS_ERROR;
        }
    }

    return LXB_STATUS_OK;
}
