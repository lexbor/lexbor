/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <lexbor/url/url.h>


typedef struct {
    const lxb_char_t *input;
    size_t           length;
    uint16_t         ipv6[8];
}
ipv6_success_t;

typedef struct {
    const lxb_char_t     *input;
    lxb_url_error_type_t error;
}
ipv6_failure_t;


static const ipv6_success_t success_entries[] = {
    {
        (const lxb_char_t *) "::",
        sizeof("::") - 1,
        {0, 0, 0, 0, 0, 0, 0, 0}
    },
    {
        (const lxb_char_t *) "::1",
        sizeof("::1") - 1,
        {0, 0, 0, 0, 0, 0, 0, 1}
    },
    {
        (const lxb_char_t *) "[::1]",
        sizeof("[::1]") - 1,
        {0, 0, 0, 0, 0, 0, 0, 1}
    },
    {
        (const lxb_char_t *) "1:2:3:4:5:6:7:8",
        sizeof("1:2:3:4:5:6:7:8") - 1,
        {1, 2, 3, 4, 5, 6, 7, 8}
    },
    {
        (const lxb_char_t *) "2001:db8::ff00:42:8329",
        sizeof("2001:db8::ff00:42:8329") - 1,
        {0x2001, 0x0db8, 0, 0, 0, 0xff00, 0x0042, 0x8329}
    },
    {
        (const lxb_char_t *) "::ffff:192.0.2.1",
        sizeof("::ffff:192.0.2.1") - 1,
        {0, 0, 0, 0, 0, 0xffff, 0xc000, 0x0201}
    },
    {
        (const lxb_char_t *) "[::1]ignored",
        5,
        {0, 0, 0, 0, 0, 0, 0, 1}
    }
};

static const ipv6_failure_t failure_entries[] = {
    {
        (const lxb_char_t *) "",
        LXB_URL_ERROR_TYPE_IPV6_TOO_FEW_PIECES
    },
    {
        (const lxb_char_t *) "[::1",
        LXB_URL_ERROR_TYPE_IPV6_UNCLOSED
    },
    {
        (const lxb_char_t *) ":",
        LXB_URL_ERROR_TYPE_IPV6_INVALID_COMPRESSION
    },
    {
        (const lxb_char_t *) "1::2::3",
        LXB_URL_ERROR_TYPE_IPV6_MULTIPLE_COMPRESSION
    },
    {
        (const lxb_char_t *) "1:2:3:4:5:6:7:8:9",
        LXB_URL_ERROR_TYPE_IPV6_TOO_MANY_PIECES
    },
    {
        (const lxb_char_t *) "1:2:3:4:5:6:7",
        LXB_URL_ERROR_TYPE_IPV6_TOO_FEW_PIECES
    },
    {
        (const lxb_char_t *) "1:2:3:4:5:6:7:g",
        LXB_URL_ERROR_TYPE_IPV6_INVALID_CODE_POINT
    },
    {
        (const lxb_char_t *) "1:2:3:4:5:6:7:1.2.3.4",
        LXB_URL_ERROR_TYPE_IPV4_IN_IPV6_TOO_MANY_PIECES
    },
    {
        (const lxb_char_t *) "::ffff:.1.2.3",
        LXB_URL_ERROR_TYPE_IPV4_IN_IPV6_INVALID_CODE_POINT
    },
    {
        (const lxb_char_t *) "::ffff:192.0.2.256",
        LXB_URL_ERROR_TYPE_IPV4_IN_IPV6_OUT_OF_RANGE_PART
    },
    {
        (const lxb_char_t *) "::ffff:192.0.2",
        LXB_URL_ERROR_TYPE_IPV4_IN_IPV6_TOO_FEW_PARTS
    }
};


TEST_BEGIN(parse_success)
{
    size_t length;
    lxb_status_t status;
    uint16_t ipv6[8];

    length = sizeof(success_entries) / sizeof(ipv6_success_t);

    for (size_t i = 0; i < length; i++) {
        memset(ipv6, 0xff, sizeof(ipv6));

        status = lxb_url_parse_host_ipv6(NULL, success_entries[i].input,
                                         success_entries[i].length, ipv6);
        test_eq(status, LXB_STATUS_OK);

        for (size_t j = 0; j < 8; j++) {
            test_eq_u_short(ipv6[j], success_entries[i].ipv6[j]);
        }
    }
}
TEST_END

TEST_BEGIN(parse_failure)
{
    size_t length;
    lxb_status_t status;
    lxb_url_parser_t parser;
    lexbor_plog_entry_t *error;

    status = lxb_url_parser_init(&parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    length = sizeof(failure_entries) / sizeof(ipv6_failure_t);

    for (size_t i = 0; i < length; i++) {
        status = lxb_url_parse_host_ipv6(
            &parser, failure_entries[i].input,
            strlen((const char *) failure_entries[i].input),
            (uint16_t[8]) {0});

        test_eq(status, LXB_STATUS_ERROR_UNEXPECTED_DATA);
        test_ne(parser.log, NULL);
        test_eq_size(lexbor_plog_length(parser.log), 1UL);

        error = lexbor_array_obj_get(&parser.log->list, 0);
        test_ne(error, NULL);
        test_eq(error->id, failure_entries[i].error);

        lxb_url_parser_clean(&parser);
    }

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

TEST_BEGIN(parse_failure_without_parser)
{
    lxb_status_t status;
    uint16_t ipv6[8];

    static const lexbor_str_t input = lexbor_str("::ffff:192.0.2.256");

    status = lxb_url_parse_host_ipv6(NULL, input.data, input.length, ipv6);
    test_eq(status, LXB_STATUS_ERROR_UNEXPECTED_DATA);
}
TEST_END

int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(parse_success);
    TEST_ADD(parse_failure);
    TEST_ADD(parse_failure_without_parser);

    TEST_RUN("lexbor/url/parse_host_ipv6");
    TEST_RELEASE();
}
