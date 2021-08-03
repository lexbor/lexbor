/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>
#include <lexbor/core/fs.h>

#include "../base.h"


static void
usage(void)
{
    fprintf(stderr, "parse_file <file>\n");
}

lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%s", (const char *) data);

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
    lxb_char_t *css;
    size_t css_len;

    if (argc != 2) {
        usage();
        FAILED("Invalid number of arguments");
    }

    css = lexbor_fs_file_easy_read((const lxb_char_t *) argv[1], &css_len);
    if (css == NULL) {
        FAILED("Failed to read CSS file");
    }

    tkz = lxb_css_syntax_tokenizer_create();
    status = lxb_css_syntax_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        PRINT("Failed to create CSS:Syntax parser");
        goto failed;
    }

    lxb_css_syntax_tokenizer_buffer_set(tkz, css, css_len);

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
    lexbor_free(css);

    return EXIT_SUCCESS;

failed:

    lxb_css_syntax_tokenizer_destroy(tkz);
    lexbor_free(css);

    return EXIT_FAILURE;
}
