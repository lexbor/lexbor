/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include "lexbor/html/tokenizer.h"


typedef struct {
    TEST_OBJ_ARG;

    lexbor_mraw_t    mraw;
    lexbor_array_t   tokens;
    lexbor_dobject_t tcmp;

    size_t           done;
}
tkz_test_ctx_t;

typedef struct {
    lexbor_str_t          data;
    lxb_html_token_type_t type;
}
tkz_test_cmp_t;


static lxb_html_token_t *
test_callback_token_done(lxb_html_tokenizer_t *tkz,
                         lxb_html_token_t *token, void *ctx)
{
    size_t length;
    tkz_test_ctx_t *entry;

    entry = ctx;

    if (lexbor_array_length(&entry->tokens) < entry->done) {
        TEST_PRINTLN("Received more tokens than expected.");
        return NULL;
    }

    tkz_test_cmp_t *test_cmp = lexbor_array_get(&entry->tokens, entry->done);

    length = token->text_end - token->text_start;

    if (length != test_cmp->data.length ||
        lexbor_str_data_ncmp(token->text_start, test_cmp->data.data, length) == false)
    {
        TEST_PRINTLN("Data in tokens are not equal:");
        TEST_PRINTLN("Have: %.*s", (int) length, (const char *) token->text_start);
        TEST_PRINTLN("Need: %.*s", (int) test_cmp->data.length,
                     (const char *) test_cmp->data.data);

        return NULL;
    }

    if (token->type != test_cmp->type) {
        TEST_PRINTLN("Tokens type not equal:");
        TEST_PRINTLN("Have: %d", token->type);
        TEST_PRINTLN("Need: %d", test_cmp->type);

        return NULL;
    }

    entry->done++;

    return token;
}

TEST_BEGIN_ARGS(tkz_init, lxb_html_tokenizer_t **tkz)
{
    lxb_status_t status;
    tkz_test_ctx_t test_ctx = {0};

    test_ctx.TEST_OBJ_NAME = TEST_OBJ_NAME;

    test_eq(lexbor_mraw_init(&test_ctx.mraw, 1024), LXB_STATUS_OK);
    test_eq(lexbor_array_init(&test_ctx.tokens, 1024), LXB_STATUS_OK);
    test_eq(lexbor_dobject_init(&test_ctx.tcmp, 1024, sizeof(tkz_test_cmp_t)),
            LXB_STATUS_OK);

    *tkz = lxb_html_tokenizer_create();
    status = lxb_html_tokenizer_init(*tkz);
    test_eq(status, LXB_STATUS_OK);

    lxb_html_tokenizer_callback_token_done_set(*tkz, test_callback_token_done,
                                               &test_ctx);
}
TEST_END

TEST_BEGIN_ARGS(tkz_process, lxb_html_tokenizer_t *tkz,
                const char *html, size_t len)
{
    lxb_status_t status;

    status = lxb_html_tokenizer_begin(tkz);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_tokenizer_chunk(tkz, (const lxb_char_t *) html, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_tokenizer_end(tkz);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

TEST_BEGIN_ARGS(tkx_end, lxb_html_tokenizer_t *tkz)
{
    lxb_html_tokenizer_destroy(tkz);
}
TEST_END

TEST_BEGIN(data_by_id)
{
    test_eq(0, 0);

//    lxb_html_tokenizer_t *tkz;

//    TEST_CALL_ARGS(tkz_init, &tkz);
    //    TEST_CALL_ARGS(tkz_process, tkz);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(data_by_id);

    TEST_RUN("lexbor/core/tokenizer");
    TEST_RELEASE();
}
