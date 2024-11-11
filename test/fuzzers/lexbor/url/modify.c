/*
 * Copyright (C) 2024 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/url/url.h>


static const char *test_fuzz_urls[] =
{
    "https://lexbor.com",
    "https://lexbor.com/docs/blah",
    "https://lexbor.com/docs/blah?la=be",
    "https://lexbor.com/docs/blah?la=be#comments",
    "https://lexbor.com/?la=be",
    "https://lexbor.com/#comments",
    "https://user@lexbor.com",
    "https://:pass@lexbor.com",
    "https://lexbor.com:8080",
    "https://lexbor.com:8080/docs/blah",
    "https://lexbor.com:8080/docs/blah?la=be",
    "https://lexbor.com:8080/docs/blah?la=be#comments",
    "https://lexbor.com:8080/?la=be",
    "https://lexbor.com:8080/#comments",
    "https://user:pass@lexbor.com:8080/docs/blah?la=be#comments",

    "hohoho://lexbor.com",
    "hohoho://lexbor.com/docs/blah",
    "hohoho://lexbor.com/docs/blah?la=be",
    "hohoho://lexbor.com/docs/blah?la=be#comments",
    "hohoho://lexbor.com/?la=be",
    "hohoho://lexbor.com/#comments",
    "hohoho://user@lexbor.com",
    "hohoho://:pass@lexbor.com",
    "hohoho://lexbor.com:8080",
    "hohoho://lexbor.com:8080/docs/blah",
    "hohoho://lexbor.com:8080/docs/blah?la=be",
    "hohoho://lexbor.com:8080/docs/blah?la=be#comments",
    "hohoho://lexbor.com:8080/?la=be",
    "hohoho://lexbor.com:8080/#comments",
    "hohoho://user:pass@lexbor.com:8080/docs/blah?la=be#comments",
};


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t length)
{
    lxb_status_t status;
    lxb_url_t *url;
    lxb_url_parser_t parser;
    const char **urls, **end;

    status = lxb_url_parser_init(&parser, NULL);
    if (status != LXB_STATUS_OK) {
        printf("Failed to init URL parser.\n");
        exit(EXIT_FAILURE);
    }

    /* Parse. */

    end = test_fuzz_urls + (sizeof(test_fuzz_urls) / sizeof(char *));

    for (urls = test_fuzz_urls; urls < end; urls++) {
        url = lxb_url_parse(&parser, NULL,
                            (const lxb_char_t *) *urls, strlen(*urls));
        if (url == NULL) {
            printf("Failed to parse URL.\n");
            exit(EXIT_FAILURE);
        }

        status = lxb_url_serialize(url, callback, NULL, false);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize URL.\n");
            exit(EXIT_FAILURE);
        }

        /* Modify. */

        (void) lxb_url_api_protocol_set(url, NULL, data, length);
        (void) lxb_url_api_username_set(url, data, length);
        (void) lxb_url_api_password_set(url, data, length);
        (void) lxb_url_api_host_set(url, NULL, data, length);
        (void) lxb_url_api_hostname_set(url, NULL, data, length);
        (void) lxb_url_api_port_set(url, NULL, data, length);
        (void) lxb_url_api_pathname_set(url, NULL, data, length);
        (void) lxb_url_api_search_set(url, NULL, data, length);
        (void) lxb_url_api_hash_set(url, NULL, data, length);

        (void) lxb_url_api_href_set(url, NULL, data, length);

        status = lxb_url_serialize(url, callback, NULL, false);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize URL.\n");
            exit(EXIT_FAILURE);
        }

        lxb_url_parser_clean(&parser);
    }

    /* Destroy. */

    lxb_url_parser_memory_destroy(&parser);
    (void) lxb_url_parser_destroy(&parser, false);

    return 0;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    return LXB_STATUS_OK;
}
