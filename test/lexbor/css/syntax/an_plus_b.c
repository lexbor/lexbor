/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>

#include <unit/test.h>


typedef lxb_css_selector_t *
(*lxb_test_flat_handler_f)(lxb_css_parser_t *parser,
                           const lxb_char_t *data, size_t length);

typedef lxb_css_selector_list_t *
(*lxb_test_list_handler_f)(lxb_css_parser_t *parser,
                           const lxb_char_t *data, size_t length);

typedef struct {
    char *test;
    char *result;
    char *errors;
}
lxb_test_entry_t;

static const lxb_test_entry_t selectors_list[] =
{
    {"odd",
     "odd",
     ""},

    {"2n+1",
     "odd",
     ""},

    {"2n-1",
     "2n-1",
     ""},

    {"2n",
     "even",
     ""},

    {"2n+0",
     "even",
     ""},

    {"2n-0",
     "even",
     ""},

    {"3n",
     "3n",
     ""},

    {"-2n",
     "-2n",
     ""},

    {"+0n+0",
     "0n",
     ""},

    {"0n+0",
     "0n",
     ""},

    {"-0n+0",
     "0n",
     ""},

    {"-0n-0",
     "0n",
     ""},

    {"+0n-0",
     "0n",
     ""},

    {"-n",
     "-n",
     ""},

    {"+n",
     "+n",
     ""},

    {"1n",
     "+n",
     ""},

    {"-1n",
     "-n",
     ""},

    {"1n-1",
     "+n-1",
     ""},

    {"1n- 1",
     "+n-1",
     ""},

    {"1n -1",
     "+n-1",
     ""},

    {"1n - 1",
     "+n-1",
     ""},

    {"1n+1",
     "+n+1",
     ""},

    {"1n+ 1",
     "+n+1",
     ""},

    {"1n + 1",
     "+n+1",
     ""},

    {"1n-+1",
     "",
     "Syntax error. An+B. Unexpected token: 1"},

    {"1n--1",
     "",
     "Syntax error. An+B. Unexpected token: 1n--1"},

    {"1n+-1",
     "",
     "Syntax error. An+B. Unexpected token: -1"},

    {"1n++1",
     "",
     "Syntax error. An+B. Unexpected token: 1"},

    {"-1n++1",
     "",
     "Syntax error. An+B. Unexpected token: 1"},

    {"1.1n+1",
     "",
     "Syntax error. An+B. Unexpected token: 1.1n"},

    {"1n+1.2",
     "",
     "Syntax error. An+B. Unexpected token: 1.2"},

    {"1n-1%",
     "",
     "Syntax error. An+B. Unexpected token: %"},

    {"+ n+1",
     "",
     "Syntax error. An+B. Unexpected token:  "},

    {"- n+1",
     "",
     "Syntax error. An+B. Unexpected token: -"},

    {"100000n+100000",
     "100000n+100000",
     ""},
};


TEST_BEGIN(lexbor_an_plus_b)
{
    char *test, *have, *need, *erro, *log;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_syntax_anb_t anb;

    size_t entries_length = sizeof(selectors_list) / sizeof(lxb_test_entry_t);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL, NULL);

    test_eq(status, LXB_STATUS_OK);

    TEST_PRINT("\n");

    for (unsigned i = 0; i < entries_length; i++) {
        test = selectors_list[i].test;
        need = selectors_list[i].result;
        erro = selectors_list[i].errors;

        TEST_PRINTLN("%u. %s", i, test);
        TEST_PRINTLN("Need Result: %s", need);
        TEST_PRINTLN("Need Log: %s", erro);

        anb = lxb_css_syntax_anb_parse(parser,
                                       (lxb_char_t *) test, strlen(test));

        if (parser->status == LXB_STATUS_OK) {
            have = (char *) lxb_css_syntax_anb_serialize_char(&anb, NULL);
        }
        else {
            have = (char *) lxb_css_syntax_anb_serialize_char(NULL, NULL);
        }

        test_eq_str(have, need);

        if (have != NULL) {
            lexbor_free(have);
        }

        log = (char *) lxb_css_log_serialize_char(parser->log, NULL, NULL, 0);
        test_eq_str(log, erro);

        if (log != NULL) {
            lexbor_free(log);
        }
    }

    lxb_css_parser_destroy(parser, true);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(lexbor_an_plus_b);

    TEST_RUN("lexbor/css/syntax/an_plus_b");
    TEST_RELEASE();
}
