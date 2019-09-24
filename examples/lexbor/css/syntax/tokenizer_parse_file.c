/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/syntax/tokenizer.h>
#include <lexbor/core/fs.h>

#include "../base.h"


static lxb_css_syntax_token_t *
token_callback_done(lxb_css_syntax_tokenizer_t *tkz,
                    lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
serialization(const lxb_char_t *data, size_t len, void *ctx);


static void
usage(void)
{
    fprintf(stderr, "tokenizer_parse_file <file>\n");
}

int
main(int argc, const char *argv[])
{
    if (argc != 2) {
        usage();
        FAILED("Invalid number of arguments");
    }

    lxb_status_t status;
    lxb_css_syntax_tokenizer_t *tkz;
    lxb_char_t *css;
    size_t css_len;

    css = lexbor_fs_file_easy_read((const lxb_char_t *) argv[1], &css_len);
    if (css == NULL) {
        FAILED("Failed to read CSS file");
    }

    /* Initialization */
    tkz = lxb_css_syntax_tokenizer_create();
    status = lxb_css_syntax_tokenizer_init(tkz);

    if (status != LXB_STATUS_OK) {
        PRINT("Failed to create CSS:Syntax parser");
        goto failed;
    }

    /* Sets callback for ready tokens */
    lxb_css_syntax_tokenizer_token_cb_set(tkz, token_callback_done, NULL);

    /* Parse CSS */
    status = lxb_css_syntax_tokenizer_parse(tkz, css, css_len);
    if (status != LXB_STATUS_OK) {
        PRINT("Failed to parse CSS");
        goto failed;
    }

    /* Destroy tokenizer */
    lxb_css_syntax_tokenizer_destroy(tkz);
    lexbor_free(css);

    return EXIT_SUCCESS;

failed:

    lxb_css_syntax_tokenizer_destroy(tkz);
    lexbor_free(css);

    return EXIT_FAILURE;
}

static lxb_css_syntax_token_t *
token_callback_done(lxb_css_syntax_tokenizer_t *tkz,
                    lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_status_t status = lxb_css_syntax_tokenizer_make_data(tkz, token);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    status = lxb_css_syntax_token_serialize_cb(token, serialization, NULL);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    return token;
}

static lxb_status_t
serialization(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, data);

    return LXB_STATUS_OK;
}
