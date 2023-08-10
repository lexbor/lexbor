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

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(deep_selectors);

    TEST_RUN("lexbor/css/stylesheet");
    TEST_RELEASE();
}
