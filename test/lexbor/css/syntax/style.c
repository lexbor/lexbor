/*
 * Copyright (C) 2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>
#include <lexbor/core/fs.h>

#include <unit/test.h>


static const lxb_char_t *lxb_test_path;


TEST_BEGIN(lexbor_style)
{
    lxb_status_t status;
    lxb_css_syntax_token_t *token;
    lxb_css_syntax_tokenizer_t *tkz;
    lxb_css_syntax_token_type_t type;

    size_t length;
    lxb_char_t *data = lexbor_fs_file_easy_read(lxb_test_path, &length);

    tkz = lxb_css_syntax_tokenizer_create();
    test_ne(tkz, NULL);

    status = lxb_css_syntax_tokenizer_init(tkz);
    test_eq(status, LXB_STATUS_OK);

    lxb_css_syntax_tokenizer_buffer_set(tkz, data, length);

    do {
        token = lxb_css_syntax_token(tkz);
        test_ne(token, NULL);

        test_ne(token->type, LXB_CSS_SYNTAX_TOKEN_UNDEF);

        type = lxb_css_syntax_token_type(token);

        lxb_css_syntax_token_consume(tkz);
    }
    while (type != LXB_CSS_SYNTAX_TOKEN__EOF);

    lxb_css_syntax_tokenizer_destroy(tkz);
    lexbor_free(data);
}
TEST_END

int
main(int argc, const char * argv[])
{
    if (argc != 2) {
        printf("Usage:\n\tcss_syntax_style <file path>\n");
        return EXIT_FAILURE;
    }

    lxb_test_path = (const lxb_char_t *) argv[1];

    TEST_INIT();

    TEST_ADD(lexbor_style);

    TEST_RUN("lexbor/css/syntax/style");
    TEST_RELEASE();
}
