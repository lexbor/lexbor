/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <unit/kv.h>

#include <lexbor/css/css.h>


TEST_BEGIN(tokenizer_queue)
{
    lxb_status_t status;
    lxb_css_syntax_token_t *token, *origin, *last;
    lxb_css_syntax_tokenizer_t *tkz;

    static const lxb_char_t css[] = "#id #class #id #class #id #class #id";
    size_t length = sizeof(css) - 1;

    tkz = lxb_css_syntax_tokenizer_create();
    status = lxb_css_syntax_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to create CSS Tokenizer");
    }

    lxb_css_syntax_tokenizer_buffer_set(tkz, css, length);

    token = lxb_css_syntax_token(tkz);
    test_ne(token, NULL);
    origin = token;
    lxb_css_syntax_token_consume(tkz);

    token = lxb_css_syntax_token(tkz);
    test_ne(token, NULL);
    test_eq(token, origin);
    lxb_css_syntax_token_consume(tkz);

    token = lxb_css_syntax_token(tkz);
    test_ne(token, NULL);
    test_eq(token, origin);
    lxb_css_syntax_token_consume(tkz);

    token = lxb_css_syntax_token(tkz);
    test_ne(token, NULL);
    test_eq(token, origin);

    token = lxb_css_syntax_token(tkz);
    test_ne(token, NULL);
    test_eq(token, origin);

    token = lxb_css_syntax_token_next(tkz);
    test_ne(token, NULL);
    test_ne(token, origin);
    test_ne(token->type, origin->type);

    token = lxb_css_syntax_token_next(tkz);
    test_ne(token, NULL);
    test_ne(token, origin);
    test_eq(token->type, origin->type);

    last = token;

    token = lxb_css_syntax_token(tkz);
    test_ne(token, NULL);
    test_eq(token, origin);

    lxb_css_syntax_token_consume(tkz);
    lxb_css_syntax_token_consume(tkz);
    lxb_css_syntax_token_consume(tkz);

    token = lxb_css_syntax_token_next(tkz);
    test_ne(token, NULL);
    test_eq(token, last);

    (void) lxb_css_syntax_tokenizer_destroy(tkz);
}
TEST_END


int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(tokenizer_queue);

    TEST_RUN("lexbor/css/syntax/tokenizer_queue");

    TEST_RELEASE();
}
