/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/url/url.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx);


int
main(int argc, const char *argv[])
{
    lxb_url_t *url, *base_url;
    lxb_status_t status;
    lxb_url_parser_t parser;
    lxb_unicode_idna_t idna;

    static const lexbor_str_t url_str = lexbor_str("/path/to/hell?id=54321#comments");
    static const lexbor_str_t base_url_str = lexbor_str("https://panda:pass@тест.com:2030/");

    status = lxb_url_parser_init(&parser, NULL);
    if (status != LXB_STATUS_OK) {
        printf("Failed to init URL parser.\n");
        return EXIT_FAILURE;
    }

    base_url = lxb_url_parse(&parser, NULL, base_url_str.data,
                             base_url_str.length);
    if (base_url == NULL) {
        printf("Failed to parse Base URL.\n");
        return EXIT_FAILURE;
    }

    lxb_url_parser_clean(&parser);

    url = lxb_url_parse(&parser, base_url, url_str.data, url_str.length);
    if (url == NULL) {
        printf("Failed to parse URL.\n");
        return EXIT_FAILURE;
    }

    lxb_url_parser_destroy(&parser, false);

    /* Output. */

    status = lxb_unicode_idna_init(&idna);
    if (status != LXB_STATUS_OK) {
        printf("Failed to init IDNA.\n");
        return EXIT_FAILURE;
    }

    printf("Base URL: %s\n", (const char *) base_url_str.data);
    printf("Relative URL: %s\n", (const char *) url_str.data);

    printf("Parsed URL: ");
    (void) lxb_url_serialize(url, callback, NULL, false);
    printf("\n");

    printf("Scheme: ");
    (void) lxb_url_serialize_scheme(url, callback, NULL);
    printf("\n");

    printf("Username: ");
    (void) lxb_url_serialize_username(url, callback, NULL);
    printf("\n");

    printf("Password: ");
    (void) lxb_url_serialize_password(url, callback, NULL);
    printf("\n");

    printf("Host (ASCII): ");
    (void) lxb_url_serialize_host(lxb_url_host(url), callback, NULL);
    printf("\n");

    printf("Host (Unicode): ");
    (void) lxb_url_serialize_host_unicode(&idna, lxb_url_host(url),
                                          callback, NULL);
    printf("\n");

    printf("Port: ");
    (void) lxb_url_serialize_port(url, callback, NULL);
    printf("\n");

    printf("Path: ");
    (void) lxb_url_serialize_path(lxb_url_path(url), callback, NULL);
    printf("\n");

    printf("Query: ");
    (void) lxb_url_serialize_query(url, callback, NULL);
    printf("\n");

    printf("Fragment: ");
    (void) lxb_url_serialize_fragment(url, callback, NULL);
    printf("\n");

    (void) lxb_unicode_idna_destroy(&idna, false);
    (void) lxb_url_memory_destroy(url);

    return EXIT_SUCCESS;
}

static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}
