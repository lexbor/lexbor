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

TEST_BEGIN(url_path_mem_error)
{
    lxb_status_t status;
    lxb_url_t *url, *before;
    lexbor_mraw_t mraw;
    lxb_url_parser_t parser;

    const lexbor_str_t input = lexbor_str("ftp:a\\aaaaaaaaaaaaaaaaaaaaaa\\\\\\");

    status = lexbor_mraw_init(&mraw, 8192);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_url_parser_init(&parser, &mraw);
    test_eq(status, LXB_STATUS_OK);

    before = NULL;

    for (size_t i = 0; i < 100; i++) {
        url = lxb_url_parse(&parser, NULL, input.data, input.length);
        test_ne(url, NULL);

        if (before != NULL) {
            lxb_url_destroy(before);
        }

        before = url;

        lxb_url_parser_clean(&parser);
    }

    lxb_url_parser_destroy(&parser, false);
    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(url_file_change_hostname)
{
    lxb_status_t status;
    lxb_url_t *url;
    lexbor_mraw_t mraw;
    lxb_url_parser_t parser;
    lxb_char_t *hostname;

    const lexbor_str_t input = lexbor_str("file:");

    status = lexbor_mraw_init(&mraw, 8192);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_url_parser_init(&parser, &mraw);
    test_eq(status, LXB_STATUS_OK);

    url = lxb_url_parse(&parser, NULL, input.data, input.length);
    test_ne(url, NULL);

    hostname = lexbor_malloc(1);

    status = lxb_url_api_hostname_set(url, &parser, hostname, 0);
    test_eq(status, LXB_STATUS_OK);

    lexbor_free(hostname);
    lxb_url_parser_destroy(&parser, false);
    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

TEST_BEGIN(url_search_params_append_after_tail_token)
{
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;
    lxb_url_search_entry_t *entry;
    lexbor_str_t str;

    static const lexbor_str_t input = lexbor_str("abc");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 1024);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, input.data, input.length);
    test_ne(sp, NULL);

    entry = lxb_url_search_params_append(sp, (const lxb_char_t *) "k", 1,
                                             (const lxb_char_t *) "v", 1);
    test_ne(entry, NULL);

    str.length = 0;
    str.data = lexbor_malloc(1024);
    test_ne(str.data, NULL);

    status = lxb_url_search_params_serialize(sp, callback, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str(str.data, "abc=&k=v");

    lexbor_free(str.data);
    lxb_url_search_params_destroy(sp);
    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(url_path_slow_path_grow)
{
    size_t i;
    lxb_status_t status;
    lxb_url_t *url;
    lexbor_mraw_t mraw;
    lxb_url_parser_t parser;
    lxb_char_t input[3 + 9 + 2000];

    memcpy(input, "http://a/", 9);
    memset(input + 9, '|', 2000);

    status = lexbor_mraw_init(&mraw, 8192);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_url_parser_init(&parser, &mraw);
    test_eq(status, LXB_STATUS_OK);

    url = lxb_url_parse(&parser, NULL, input, 9 + 2000);
    test_ne(url, NULL);

    test_eq(url->path.str.length, (size_t) (1 + 2000));
    test_eq(url->path.str.data[0], (lxb_char_t) '/');

    for (i = 1; i < url->path.str.length; i++) {
        test_eq(url->path.str.data[i], (lxb_char_t) '|');
    }

    lxb_url_parser_destroy(&parser, false);
    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(url_clone);
    TEST_ADD(url_path_mem_error);
    TEST_ADD(url_file_change_hostname);
    TEST_ADD(url_search_params_append_after_tail_token);
    TEST_ADD(url_path_slow_path_grow);

    TEST_RUN("lexbor/url/other");
    TEST_RELEASE();
}
