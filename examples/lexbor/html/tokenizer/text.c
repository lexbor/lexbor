/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tokenizer.h"


#define FAILED(...)                                                            \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    while (0)


static lxb_html_token_t *
token_callback(lxb_html_tokenizer_t *tkz, lxb_html_token_t *token, void *ctx)
{
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    lexbor_str_t str = {0};

    /* Skip all not #text tokens */
    if (token->tag_id != LXB_TAG__TEXT) {
        return token;
    }

    mraw = lxb_html_tokenizer_mraw(tkz);

    status = lxb_html_token_make_data(token, &str, mraw);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to make data from token");
    }

    printf("%s", str.data);

    lexbor_str_destroy(&str, mraw, false);

    return token;
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_html_tokenizer_t *tkz;

    const lxb_char_t data[] = "<div>Hi<span> my </span>friend</div>";

    tkz = lxb_html_tokenizer_create();
    status = lxb_html_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create tokenizer object");
    }

    /* Without copying input buffer */
    lxb_html_tokenizer_opt_set(tkz, LXB_HTML_TOKENIZER_OPT_WO_COPY);
    /* Set callback for token */
    lxb_html_tokenizer_callback_token_done_set(tkz, token_callback, NULL);

    status = lxb_html_tokenizer_begin(tkz);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to prepare tokenizer object for parsing");
    }

    status = lxb_html_tokenizer_chunk(tkz, data, (sizeof(data) - 1));
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse the html data");
    }

    status = lxb_html_tokenizer_end(tkz);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to ending of parsing the html data");
    }

    printf("\n");

    lxb_html_tokenizer_destroy(tkz);

    return 0;
}
