/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <lexbor/url/url.h>


#define URL_IDNA_LABEL_COUNT 300
#define URL_IDNA_ASCII_LABEL "xn--4caaaaaaaaaa"


static int url_idna_fill;


static void *
url_idna_malloc(size_t size)
{
    void *data;

    data = malloc(size);
    if (data != NULL) {
        memset(data, url_idna_fill, size);
    }

    return data;
}

static lxb_status_t
url_idna_callback(const lxb_char_t *data, size_t length, void *ctx)
{
    lexbor_str_t *str = ctx;

    if (length > URL_IDNA_LABEL_COUNT * sizeof(URL_IDNA_ASCII_LABEL)
                 - str->length - 1)
    {
        return LXB_STATUS_ERROR_SMALL_BUFFER;
    }

    memcpy(&str->data[str->length], data, length);

    str->length += length;
    str->data[str->length] = '\0';

    return LXB_STATUS_OK;
}

TEST_BEGIN_ARGS(url_idna_large, int fill)
{
    size_t i, input_length, expected_length;
    lxb_status_t status, restore_status;
    lxb_url_t *url;
    lxb_url_parser_t parser;
    lexbor_str_t str;
    lxb_char_t input[8 + URL_IDNA_LABEL_COUNT * 21 + 1];
    lxb_char_t output[URL_IDNA_LABEL_COUNT * sizeof(URL_IDNA_ASCII_LABEL)];
    lxb_char_t expected[URL_IDNA_LABEL_COUNT * sizeof(URL_IDNA_ASCII_LABEL)];

    /* Ten U+00E4 characters per label; the ASCII host has 5099 bytes. */
    static const lxb_char_t label[] =
        "\xC3\xA4\xC3\xA4\xC3\xA4\xC3\xA4\xC3\xA4"
        "\xC3\xA4\xC3\xA4\xC3\xA4\xC3\xA4\xC3\xA4";

    memcpy(input, "https://", 8);
    input_length = 8;
    expected_length = 0;

    for (i = 0; i < URL_IDNA_LABEL_COUNT; i++) {
        if (i != 0) {
            input[input_length++] = '.';
            expected[expected_length++] = '.';
        }

        memcpy(input + input_length, label, sizeof(label) - 1);
        input_length += sizeof(label) - 1;

        memcpy(expected + expected_length, URL_IDNA_ASCII_LABEL,
               sizeof(URL_IDNA_ASCII_LABEL) - 1);
        expected_length += sizeof(URL_IDNA_ASCII_LABEL) - 1;
    }

    input[input_length++] = '/';
    input[input_length] = '\0';
    expected[expected_length] = '\0';

    status = lxb_url_parser_init(&parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    /* Make a missing copy into the grown IDNA buffer deterministic. */
    url_idna_fill = fill;
    status = lexbor_memory_setup(url_idna_malloc, realloc, calloc, free);
    url = lxb_url_parse(&parser, NULL, input, input_length);
    restore_status = lexbor_memory_setup(malloc, realloc, calloc, free);

    test_eq(status, LXB_STATUS_OK);
    test_eq(restore_status, LXB_STATUS_OK);
    test_ne(url, NULL);

    test_eq(url->host.type, LXB_URL_HOST_TYPE_DOMAIN);
    test_eq_str_n(url->host.u.domain.data, url->host.u.domain.length, expected,
                  expected_length);

    str.data = output;
    str.length = 0;

    status = lxb_url_serialize_host(&url->host, url_idna_callback, &str);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(str.data, str.length, expected, expected_length);

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

TEST_BEGIN(url_idna_large_ascii_memory)
{
    TEST_CALL_ARGS(url_idna_large, 'A');
}
TEST_END

TEST_BEGIN(url_idna_large_zero_memory)
{
    TEST_CALL_ARGS(url_idna_large, 0);
}
TEST_END

int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(url_idna_large_ascii_memory);
    TEST_ADD(url_idna_large_zero_memory);

    TEST_RUN("lexbor/url/idna_large");
    TEST_RELEASE();
}
