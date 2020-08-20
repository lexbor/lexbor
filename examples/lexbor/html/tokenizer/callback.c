/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <inttypes.h>

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
    bool is_close;
    const lxb_char_t *name;
    lexbor_hash_t *tags = lxb_html_tokenizer_tags(tkz);

    name = lxb_tag_name_by_id(tags, token->tag_id, NULL);
    if (name == NULL) {
        FAILED("Failed to get token name");
    }

    is_close = token->type & LXB_HTML_TOKEN_TYPE_CLOSE;

    printf("Tag name: %s; Tag id: %" PRIxPTR "; Is close: %s\n", name,
           token->tag_id, (is_close ? "true" : "false"));

    return token;
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_html_tokenizer_t *tkz;

    const lxb_char_t data[] = "<div><span>test</span></div>";

    printf("HTML:\n%s\n\n", (char *) data);
    printf("Result:\n");

    tkz = lxb_html_tokenizer_create();
    status = lxb_html_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create tokenizer object");
    }

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

    lxb_html_tokenizer_destroy(tkz);

    return 0;
}
