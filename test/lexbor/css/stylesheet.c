/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/css/css.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    return LXB_STATUS_OK;
}

TEST_BEGIN(deep_selectors)
{
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;

    const lexbor_str_t input = lexbor_str(
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not(:not("
    ":not(:not(:not(:not(:not(:not(:not(abc");

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_parse(parser, input.data, input.length);
    test_ne(sst, NULL);

    status = lxb_css_rule_serialize(sst->root, callback, NULL);
    test_eq(status, LXB_STATUS_OK);

    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
}
TEST_END

TEST_BEGIN(prepared_token)
{
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;

    const lexbor_str_t input = lexbor_str(
    "2.2-.2-.2-.2-2-.2-.2-.2-.2-.2-.2-2-2-.2-.2-.2-2-.2-.2-.2-."
    "2-.2-.2-2-.2-.2-.2-2.2-.2-.2-.2-.2-.2-2-22-.2-.2-.2-2-.2-."
    "2-.2-.2-.2-.22-.2-.2-.2-2-.2-.2-.2-.2-.2-.2-2-.2-.2-.2-2.2"
    "-.2-.2-.2-.2-.2-2-22-.2-.2-.2-2-.2-.2-.2-.2-.2-.2-2--2-.2-.22-.2.2.23");

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_parse(parser, input.data, input.length);
    test_ne(sst, NULL);

    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
}
TEST_END

TEST_BEGIN(eof_offset)
{
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;

    const lexbor_str_t input = lexbor_str("url((\\");

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_parse(parser, input.data, input.length);
    test_ne(sst, NULL);

    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
}
TEST_END

TEST_BEGIN(colon_lookup)
{
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;

    const lexbor_str_t input = lexbor_str("div {width\\\n: 100%");

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_parse(parser, input.data, input.length);
    test_ne(sst, NULL);

    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(deep_selectors);
    TEST_ADD(prepared_token);
    TEST_ADD(eof_offset);
    TEST_ADD(colon_lookup);

    TEST_RUN("lexbor/css/stylesheet");
    TEST_RELEASE();
}
