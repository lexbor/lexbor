/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>

#include "../base.h"


/* Specially small buffer for demonstration. */
#define BUFFER_SIZE 32


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%s", (const char *) data);

    return LXB_STATUS_OK;
}

lxb_status_t
chunk_cb(lxb_css_syntax_tokenizer_t *tkz, const lxb_char_t **data,
         const lxb_char_t **end, void *ctx)
{
    size_t size;
    lxb_char_t *buff = ctx;

    size = fread((char *) buff, 1, BUFFER_SIZE, stdin);
    if (size != BUFFER_SIZE) {
        if (feof(stdin)) {
            tkz->eof = true;
        }
        else {
            return EXIT_FAILURE;
        }
    }

    *data = buff;
    *end = buff + size;

    return LXB_STATUS_OK;
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_css_syntax_token_t *token;
    lxb_css_syntax_tokenizer_t *tkz;
    lxb_css_syntax_token_type_t type;
    const lxb_char_t *name;
    char inbuf[BUFFER_SIZE];

    tkz = lxb_css_syntax_tokenizer_create();
    status = lxb_css_syntax_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        PRINT("Failed to create CSS:Syntax parser");
        goto failed;
    }

    lxb_css_syntax_tokenizer_chunk_cb_set(tkz, chunk_cb, inbuf);

    do {
        token = lxb_css_syntax_token(tkz);
        if (token == NULL) {
            PRINT("Failed to parse CSS");
            goto failed;
        }

        name = lxb_css_syntax_token_type_name_by_id(token->type);
        printf("%s: ", (const char *) name);

        lxb_css_syntax_token_serialize(token, callback, NULL);
        printf("\n");

        type = lxb_css_syntax_token_type(token);

        lxb_css_syntax_token_consume(tkz);
    }
    while (type != LXB_CSS_SYNTAX_TOKEN__EOF);

    lxb_css_syntax_tokenizer_destroy(tkz);

    return EXIT_SUCCESS;

failed:

    lxb_css_syntax_tokenizer_destroy(tkz);

    return EXIT_FAILURE;
}
