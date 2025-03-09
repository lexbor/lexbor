/*
 * Copyright (C) 2020-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    while (*data != 0x00) {
        data++;
    }

    return LXB_STATUS_OK;
}

#ifndef LEXBOR_HAVE_FUZZER
int
main(int argc, const char *argv[])
{
    return EXIT_SUCCESS;
}
#endif

static void
check_raw(lxb_css_syntax_token_t *token)
{
    volatile lxb_char_t ch;
    const lxb_char_t *p, *end;

    p = lxb_css_syntax_token_base(token)->begin;
    end = p + lxb_css_syntax_token_base(token)->length;

    while (p < end) {
        ch = *p++;
        ch++;
    }

    if (token->type == LXB_CSS_SYNTAX_TOKEN_DIMENSION) {
        p = lxb_css_syntax_token_dimension_string(token)->base.begin;
        end = p + lxb_css_syntax_token_dimension_string(token)->base.length;

        while (p < end) {
            ch = *p++;
            ch++;
        }
    }
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t length)
{
    lxb_status_t status;
    lxb_css_syntax_token_t *token;
    lxb_css_syntax_tokenizer_t *tkz;
    lxb_css_syntax_token_type_t type;

    tkz = lxb_css_syntax_tokenizer_create();
    if (tkz == NULL) {
        return EXIT_FAILURE;
    }

    status = lxb_css_syntax_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    lxb_css_syntax_tokenizer_with_unicode_range(tkz, true);
    tkz->with_comment = true;

    lxb_css_syntax_tokenizer_buffer_set(tkz, data, length);

    do {
        token = lxb_css_syntax_token(tkz);
        if (token == NULL) {
            return EXIT_FAILURE;
        }

        check_raw(token);

        if (lxb_css_syntax_token_type(token) == LXB_CSS_SYNTAX_TOKEN_UNDEF) {
            return EXIT_FAILURE;
        }

        lxb_css_syntax_token_serialize(token, callback, NULL);

        type = lxb_css_syntax_token_type(token);

        lxb_css_syntax_token_consume(tkz);
    }
    while (type != LXB_CSS_SYNTAX_TOKEN__EOF);

    lxb_css_syntax_tokenizer_destroy(tkz);

    return EXIT_SUCCESS;
}
