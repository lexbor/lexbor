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

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(use_log_after_free);
    TEST_ADD(empty_host_after_userinfo);

    TEST_RUN("lexbor/url/errors");
    TEST_RELEASE();
}
