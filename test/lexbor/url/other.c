/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <lexbor/url/url.h>


typedef struct {
    const char *base;
    const char *input;
    const char *path;
    size_t     length;
}
test_path_length_t;


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

TEST_BEGIN(url_path_length)
{
    size_t i, len;
    lxb_status_t status;
    lxb_url_t *url, *base;
    lxb_url_parser_t parser;
    const test_path_length_t *entry;

    static const test_path_length_t entries[] = {
        {NULL, "https://lexbor.com", "/", 1},
        {NULL, "https://lexbor.com/", "/", 1},
        {NULL, "https://lexbor.com/a", "/a", 1},
        {NULL, "https://lexbor.com/a/", "/a/", 2},
        {NULL, "https://lexbor.com//", "//", 2},
        {NULL, "https://lexbor.com/a\\", "/a/", 2},
        {NULL, "https://lexbor.com/a/.", "/a/", 2},
        {NULL, "https://lexbor.com/a/./", "/a/", 2},
        {NULL, "https://lexbor.com/a/..", "/", 1},
        {NULL, "https://lexbor.com/a/../..", "/", 1},
        {NULL, "https://lexbor.com/a/../../..?x", "/", 1},
        {NULL, "https://lexbor.com/a/b/../../../c", "/c", 1},
        {NULL, "https://lexbor.com/a/..?x", "/", 1},
        {NULL, "https://lexbor.com/a/.#x", "/a/", 2},
        {NULL, "https://lexbor.com//./c", "//c", 2},
        {NULL, "https://lexbor.com//\u00E9", "//%C3%A9", 2},
        {NULL, "https://lexbor.com/\u00E9/", "/%C3%A9/", 2},
        {NULL, "https://lexbor.com/\u00E9/.", "/%C3%A9/", 2},
        {NULL, "https://lexbor.com/\u00E9/a/..", "/%C3%A9/", 2},
        {NULL, "https://lexbor.com/\u00E9/a/../..", "/", 1},
        {NULL, "https://lexbor.com/\u00E9/a/..?x", "/%C3%A9/", 2},
        {NULL, "https://lexbor.com/a/b/../../../c/\u00E9/../../x", "/x", 1},
        {NULL, "https://lexbor.com/%2/../x", "/x", 1},
        {NULL, "https://lexbor.com/%2/..#frag", "/", 1},
        {NULL, "https://lexbor.com/a/%2/../../x", "/x", 1},
        {NULL, "https://lexbor.com/%2/", "/%2/", 2},
        {NULL, "https://lexbor.com/%?q", "/%", 1},
        {NULL, "https://lexbor.com/%#f", "/%", 1},
        {NULL, "https://lexbor.com//%2/../x", "//x", 2},
        {NULL, "https://lexbor.com/\u00E9/%2/../x", "/%C3%A9/x", 2},
        {NULL, "https://lexbor.com/.%/../x", "/x", 1},
        {NULL, "file:///C:/..", "/C:/", 2},
        {NULL, "file:///C:/%2/../..", "/C:/", 2},
        {NULL, "file:///C:/a/../..", "/C:/", 2},
        {NULL, "file:\\\\", "/", 1},
        {NULL, "file:\\\\\\\\", "//", 2},
        {"file:///C:/a/b", "/", "/C:/", 2},
        {"file:///C:/a/b", "/..", "/C:/", 2},
        {"https://lexbor.com/b/c/", ".", "/b/c/", 3},
        {"https://lexbor.com/b/c/", "..", "/b/", 2},
        {"https://lexbor.com/b/c/", "../../..", "/", 1},
        {"https://lexbor.com/b//", "./x", "/b//x", 3},
        {"https://lexbor.com/b/c/", "%2/../x", "/b/c/x", 3},
        {"non-spec:/p", "..//path", "//path", 2}
    };

    status = lxb_url_parser_init(&parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    for (i = 0; i < sizeof(entries) / sizeof(test_path_length_t); i++) {
        entry = &entries[i];
        base = NULL;

        if (entry->base != NULL) {
            len = strlen(entry->base);

            base = lxb_url_parse(&parser, NULL, (const lxb_char_t *) entry->base,
                                 len);
            test_ne(base, NULL);

            lxb_url_parser_clean(&parser);
        }

        len = strlen(entry->input);

        url = lxb_url_parse(&parser, base, (const lxb_char_t *) entry->input,
                            len);
        test_ne(url, NULL);

        test_eq_str(url->path.str.data, entry->path);
        test_eq_size(url->path.length, entry->length);

        lxb_url_parser_clean(&parser);
    }

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

TEST_BEGIN(url_pathname_invalid_percent)
{
    size_t i;
    lxb_status_t status;
    lxb_url_t *url;
    lxb_url_parser_t parser;
    const test_path_length_t *entry;

    static const lexbor_str_t input = lexbor_str("https://lexbor.com/a?q#f");
    static const test_path_length_t entries[] = {
        {NULL, "/%2/../x", "/x", 1},
        {NULL, "/a/%2/../../x", "/x", 1},
        {NULL, "/%2/", "/%2/", 2},
        {NULL, "/%2/..#frag", "/%2/..%23frag", 2},
        {NULL, "/%2/..?query", "/%2/..%3Fquery", 2},
        {NULL, "/%\u00E9/../x", "/x", 1}
    };

    status = lxb_url_parser_init(&parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    url = lxb_url_parse(&parser, NULL, input.data, input.length);
    test_ne(url, NULL);

    for (i = 0; i < sizeof(entries) / sizeof(test_path_length_t); i++) {
        entry = &entries[i];
        lxb_url_parser_clean(&parser);

        status = lxb_url_api_pathname_set(url, &parser,
                                          (const lxb_char_t *) entry->input,
                                          strlen(entry->input));
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(url->path.str.data, entry->path);
        test_eq_size(url->path.length, entry->length);
        test_eq_str(url->query.data, "q");
        test_eq_str(url->fragment.data, "f");
    }

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
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
    TEST_ADD(url_path_length);
    TEST_ADD(url_pathname_invalid_percent);

    TEST_RUN("lexbor/url/other");
    TEST_RELEASE();
}
