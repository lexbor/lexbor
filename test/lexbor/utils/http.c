/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <string.h>

#include <unit/test.h>

#include <lexbor/utils/http.h>


static lxb_status_t
parse_buf(lxb_utils_http_t *http, const char *data)
{
    const lxb_char_t *p = (const lxb_char_t *) data;

    return lxb_utils_http_parse(http, &p, p + strlen(data));
}

TEST_BEGIN(serialize_crlf)
{
    lxb_status_t status;
    lexbor_str_t str;
    lxb_utils_http_t http;
    const lxb_utils_http_field_t *field;

    test_eq(lxb_utils_http_init(&http, NULL), LXB_STATUS_OK);

    status = parse_buf(&http, "HTTP/1.1 200 OK\r\nX-Foo: bar\r\n\r\n");
    test_eq(status, LXB_STATUS_OK);

    field = lxb_utils_http_header_field(&http, (const lxb_char_t *) "X-Foo",
                                        5, 0);
    test_ne(field, NULL);
    test_eq(field->value.length, 3);
    test_eq(memcmp(field->value.data, "bar", 3), 0);

    str.data = NULL;
    str.length = 0;
    test_eq(lxb_utils_http_header_serialize(&http, &str), LXB_STATUS_OK);
    test_eq(str.length, 12);
    test_eq(memcmp(str.data, "X-Foo: bar\r\n", 12), 0);

    lxb_utils_http_destroy(&http, false);
}
TEST_END

TEST_BEGIN(reject_cr_in_value)
{
    lxb_status_t status;
    lxb_utils_http_t http;

    test_eq(lxb_utils_http_init(&http, NULL), LXB_STATUS_OK);

    status = parse_buf(&http,
                       "HTTP/1.1 200 OK\r\nX-Foo: bar\rInjected: evil\r\n\r\n");
    test_eq(status, LXB_STATUS_ABORTED);

    lxb_utils_http_destroy(&http, false);
}
TEST_END

TEST_BEGIN(htab_in_value)
{
    lxb_status_t status;
    lxb_utils_http_t http;
    const lxb_utils_http_field_t *field;

    test_eq(lxb_utils_http_init(&http, NULL), LXB_STATUS_OK);

    status = parse_buf(&http, "HTTP/1.1 200 OK\r\nX-Foo: bar\tbaz\r\n\r\n");
    test_eq(status, LXB_STATUS_OK);

    field = lxb_utils_http_header_field(&http, (const lxb_char_t *) "X-Foo",
                                        5, 0);
    test_ne(field, NULL);
    test_eq(field->value.length, 7);
    test_eq(memcmp(field->value.data, "bar\tbaz", 7), 0);

    lxb_utils_http_destroy(&http, false);
}
TEST_END

int
main(int argc, const char * argv[])
{
    (void) argc;
    (void) argv;

    TEST_INIT();

    TEST_ADD(serialize_crlf);
    TEST_ADD(reject_cr_in_value);
    TEST_ADD(htab_in_value);

    TEST_RUN("lexbor/utils/http");
    TEST_RELEASE();
}
