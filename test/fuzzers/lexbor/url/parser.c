/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/url/url.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t length)
{
    lxb_status_t status;
    lxb_url_t *url;
    lxb_url_t *base_url;
    lxb_url_parser_t parser;

    static const lexbor_str_t burl = lexbor_str("https://lexbor.com/docs/blah");
    static const lexbor_str_t burl_shm = lexbor_str("hohoho://lexbor.com/docs/blah");
    static const lexbor_str_t burl_full = lexbor_str("hohoho://user:login@lexbor.com:1020/docs/blah?la=be#comments");

    status = lxb_url_parser_init(&parser, NULL);
    if (status != LXB_STATUS_OK) {
        printf("Failed to init URL parser.\n");
        return EXIT_FAILURE;
    }

    /* Parse. */

    url = lxb_url_parse(&parser, NULL, data, length);
    if (url == NULL) {
        /* It is normal. Can be. */
    }

    if (url != NULL) {
        status = lxb_url_serialize(url, callback, NULL, false);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize URL.\n");
            return EXIT_FAILURE;
        }
    }

    /* Parse with base URL. */

    lxb_url_parser_clean(&parser);

    base_url = lxb_url_parse(&parser, NULL, burl.data, burl.length);
    if (base_url == NULL) {
        printf("Failed to parse Base URL.\n");
        return EXIT_FAILURE;
    }

    lxb_url_parser_clean(&parser);

    url = lxb_url_parse(&parser, base_url, data, length);
    if (url == NULL) {
        /* It is normal. Can be. */
    }

    if (url != NULL) {
        status = lxb_url_serialize(url, callback, NULL, false);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize URL.\n");
            return EXIT_FAILURE;
        }
    }

    /* Parse with base URL scheme not special. */

    lxb_url_parser_clean(&parser);

    base_url = lxb_url_parse(&parser, NULL, burl_shm.data, burl_shm.length);
    if (base_url == NULL) {
        printf("Failed to parse Base URL scheme not special.\n");
        return EXIT_FAILURE;
    }

    lxb_url_parser_clean(&parser);

    url = lxb_url_parse(&parser, base_url, data, length);
    if (url == NULL) {
        /* It is normal. Can be. */
    }

    if (url != NULL) {
        status = lxb_url_serialize(url, callback, NULL, false);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize URL.\n");
            return EXIT_FAILURE;
        }
    }

    /* Parse with full base URL. */

    lxb_url_parser_clean(&parser);

    base_url = lxb_url_parse(&parser, NULL, burl_full.data, burl_full.length);
    if (base_url == NULL) {
        printf("Failed to parse full Base URL.\n");
        return EXIT_FAILURE;
    }

    lxb_url_parser_clean(&parser);

    url = lxb_url_parse(&parser, base_url, data, length);
    if (url == NULL) {
        /* It is normal. Can be. */
    }

    if (url != NULL) {
        status = lxb_url_serialize(url, callback, NULL, false);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize URL.\n");
            return EXIT_FAILURE;
        }
    }

    lxb_url_parser_memory_destroy(&parser);
    (void) lxb_url_parser_destroy(&parser, false);

    return EXIT_SUCCESS;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    return LXB_STATUS_OK;
}
