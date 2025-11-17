/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <lexbor/url/url.h>


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lexbor_str_t *str = ctx;

    memcpy(&str->data[str->length], data, len);

    str->length += len;
    str->data[str->length] = '\0';

    return LXB_STATUS_OK;
}

TEST_BEGIN(url_clone)
{
    lxb_status_t status;
    lxb_url_t *url, *cloned;
    lxb_url_parser_t parser;
    lexbor_str_t str;

    static const lexbor_str_t input = lexbor_str("https://192.168.0.1/");

    status = lxb_url_parser_init(&parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    url = lxb_url_parse(&parser, NULL, input.data, input.length);
    test_ne(url, NULL);

    cloned = lxb_url_clone(parser.mraw, url);
    test_ne(cloned, NULL);

    str.length = 0;
    str.data = lexbor_malloc(1024);
    test_ne(str.data, NULL);

    status = lxb_url_serialize(cloned, callback, &str, false);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str(str.data, input.data);

    lexbor_free(str.data);

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(url_clone);

    TEST_RUN("lexbor/url/other");
    TEST_RELEASE();
}
