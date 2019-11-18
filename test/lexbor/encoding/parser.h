/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/encoding/encoding.h"


#define LEXBOR_STR_RES_MAP_HEX
#define LEXBOR_STR_RES_MAP_NUM
#include "lexbor/core/str_res.h"


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

            return LXB_ENCODING_ERROR_CODEPOINT;
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
        if (num == LXB_ENCODING_ERROR_CODEPOINT) {
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
    if (entry->cp == LXB_ENCODING_ERROR_CODEPOINT) {
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
