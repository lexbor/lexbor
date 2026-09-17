/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <lexbor/url/url.h>


typedef struct {
    const lxb_char_t *input;
}
empty_host_error_t;


TEST_BEGIN(use_log_after_free)
{
    lxb_status_t status;
    lxb_url_parser_t parser;
    lexbor_plog_entry_t *error;
    static const lexbor_str_t input = lexbor_str("x://:\n/");

    status = lxb_url_parser_init(&parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    (void) lxb_url_parse(&parser, NULL, input.data, input.length);

    while ((error = lexbor_array_obj_pop(&parser.log->list)) != NULL) {
        printf("error: %d at %s\n", error->id, error->data);
    }

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

TEST_BEGIN(empty_host_after_userinfo)
{
    size_t length;
    lxb_status_t status;
    lxb_url_parser_t parser;
    lexbor_plog_entry_t *error;
    static const empty_host_error_t entries[] = {
        {(const lxb_char_t *) "https://user:pass@"},
        {(const lxb_char_t *) "https://user@"},
        {(const lxb_char_t *) "https://@"},
        {(const lxb_char_t *) "https://user:pass@/"},
        {(const lxb_char_t *) "https://user:pass@?x"},
        {(const lxb_char_t *) "https://user:pass@#x"}
    };

    status = lxb_url_parser_init(&parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    for (size_t i = 0; i < sizeof(entries) / sizeof(empty_host_error_t); i++) {
        length = strlen((const char *) entries[i].input);

        status = lxb_url_parse_basic(&parser, NULL, NULL, entries[i].input,
                                     length, LXB_URL_STATE__UNDEF,
                                     LXB_ENCODING_AUTO);
        test_eq(status, LXB_STATUS_ERROR_UNEXPECTED_DATA);
        test_ne(parser.log, NULL);
        test_eq_size(lexbor_plog_length(parser.log), 2UL);

        error = lexbor_array_obj_get(&parser.log->list, 0);
        test_ne(error, NULL);
        test_eq(error->id, LXB_URL_ERROR_TYPE_INVALID_CREDENTIALS);

        error = lexbor_array_obj_get(&parser.log->list, 1);
        test_ne(error, NULL);
        test_eq(error->id, LXB_URL_ERROR_TYPE_HOST_MISSING);

        lxb_url_parser_clean(&parser);
    }

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

TEST_BEGIN(owned_error_context)
{
    lxb_url_parser_t parser;
    lxb_url_t *url;
    lexbor_plog_entry_t *error;
    lxb_char_t *input;
    size_t length;
    static const struct {
        const char           *input;
        const char           *context;
        lxb_url_error_type_t  type;
    } entries[] = {
        {"http://1.2.3.4.5/", "5", LXB_URL_ERROR_TYPE_IPV4_TOO_MANY_PARTS},
        {"http://0x7f.0.0.1/", "0x7f.0.0.1", LXB_URL_ERROR_TYPE_IPV4_NON_DECIMAL_PART},
        {"http://example.org/%", "%", LXB_URL_ERROR_TYPE_INVALID_URL_UNIT},
        {"http://example.org/\n%", "%", LXB_URL_ERROR_TYPE_INVALID_URL_UNIT},
        {"http://example.org/ ", " ", LXB_URL_ERROR_TYPE_INVALID_URL_UNIT},
        {"http://exa%23mple.org/", "#mple.org",
         LXB_URL_ERROR_TYPE_DOMAIN_INVALID_CODE_POINT}
    };

    test_eq(lxb_url_parser_init(&parser, NULL), LXB_STATUS_OK);

    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        length = strlen(entries[i].input);
        input = lexbor_malloc(length);
        test_ne(input, NULL);
        memcpy(input, entries[i].input, length);

        url = lxb_url_parse(&parser, NULL, input, length);
        lexbor_free(input);
        lxb_url_destroy(url);

        /* The log must also survive destruction of the URL arena. */
        lxb_url_parser_memory_destroy(&parser);
        test_ne(parser.log, NULL);
        error = lexbor_array_obj_pop(&parser.log->list);
        test_ne(error, NULL);
        test_eq(error->id, entries[i].type);
        test_eq_str(error->data, entries[i].context);

        /* Popped entries must not prevent their contexts being reclaimed. */
        lxb_url_parser_clean(&parser);
        lxb_url_parser_destroy(&parser, false);
        test_eq(lxb_url_parser_init(&parser, NULL), LXB_STATUS_OK);
    }

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

TEST_BEGIN(ipv4_temporary_memory)
{
    lxb_url_parser_t parser;
    lxb_url_t *url;
    static const lexbor_str_t input = lexbor_str("http://127.0.0.1/");

    test_eq(lxb_url_parser_init(&parser, NULL), LXB_STATUS_OK);

    for (size_t i = 0; i < 100; i++) {
        url = lxb_url_parse(&parser, NULL, input.data, input.length);
        test_ne(url, NULL);
        lxb_url_destroy(url);
        lxb_url_parser_clean(&parser);
        test_eq_size(lexbor_mraw_reference_count(parser.mraw), 0UL);
    }

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

TEST_BEGIN(many_error_contexts)
{
    lxb_url_parser_t parser;
    lxb_url_t *url;
    lexbor_plog_entry_t *error;
    lxb_char_t input[4096];
    const lxb_char_t *first;
    static const char prefix[] = "http://example.org/#";
    size_t prefix_length = sizeof(prefix) - 1;

    memcpy(input, prefix, prefix_length);
    memset(input + prefix_length, '^', sizeof(input) - prefix_length);
    test_eq(lxb_url_parser_init(&parser, NULL), LXB_STATUS_OK);
    url = lxb_url_parse(&parser, NULL, input, sizeof(input));
    test_ne(url, NULL);
    test_eq_size(lexbor_plog_length(parser.log), sizeof(input) - prefix_length);

    error = lexbor_array_obj_get(&parser.log->list, 0);
    first = error->data;
    test_eq_size(strlen((const char *) first), sizeof(input) - prefix_length);

    for (size_t i = 0; i < lexbor_plog_length(parser.log); i++) {
        error = lexbor_array_obj_get(&parser.log->list, i);
        test_eq(error->data, first + i);
    }

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(use_log_after_free);
    TEST_ADD(empty_host_after_userinfo);
    TEST_ADD(owned_error_context);
    TEST_ADD(ipv4_temporary_memory);
    TEST_ADD(many_error_contexts);

    TEST_RUN("lexbor/url/errors");
    TEST_RELEASE();
}
