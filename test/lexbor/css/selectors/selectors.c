
/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>

#include <unit/test.h>

typedef lxb_css_selector_list_t *
(*lxb_test_list_handler_f)(lxb_css_parser_t *parser,
                           const lxb_char_t *data, size_t length);

typedef struct {
    char *test;
    char *result;
    char *errors;
    lxb_test_list_handler_f handler;
}
lxb_test_entry_t;

static const lxb_test_entry_t selectors_list[] =
{
    {"a",
     "a",
     "",
     lxb_css_selectors_parse},

    {".super",
     ".super",
     "",
     lxb_css_selectors_parse},

    {" .super ",
     ".super",
     "",
     lxb_css_selectors_parse},

    {".super 1%",
     "",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {"#hash",
     "#hash",
     "",
     lxb_css_selectors_parse},

    {"*|*",
     "*|*",
     "",
     lxb_css_selectors_parse},

    {"*|div",
     "*|div",
     "",
     lxb_css_selectors_parse},

    {"|div",
     "*|div",
     "",
     lxb_css_selectors_parse},

    {"* |div",
     "* *|div",
     "",
     lxb_css_selectors_parse},

    {"html|div",
     "html|div",
     "",
     lxb_css_selectors_parse},

    {"html |div",
     "html *|div",
     "",
     lxb_css_selectors_parse},

    {"[refs='link']",
     "[refs=\"link\"]",
     "",
     lxb_css_selectors_parse},

    {" [ refs = 'link' ] ",
     "[refs=\"link\"]",
     "",
     lxb_css_selectors_parse},

    {"[refs=\"link\"]",
     "[refs=\"link\"]",
     "",
     lxb_css_selectors_parse},

    {"[refs='link'i]",
     "[refs=\"link\"i]",
     "",
     lxb_css_selectors_parse},

    {"[refs='link' i]",
     "[refs=\"link\"i]",
     "",
     lxb_css_selectors_parse},

    {"[refs='link' i ]",
     "[refs=\"link\"i]",
     "",
     lxb_css_selectors_parse},

    {"[refs]",
     "[refs]",
     "",
     lxb_css_selectors_parse},

    {"[refs=link]",
     "[refs=\"link\"]",
     "",
     lxb_css_selectors_parse},

    {"[refs=link i]",
     "[refs=\"link\"i]",
     "",
     lxb_css_selectors_parse},

    {"[refs='a\"b\"c']",
     "[refs=\"a\\000022b\\000022c\"]",
     "",
     lxb_css_selectors_parse},

    {"[refs=link I]",
     "",
     "Syntax error. Selectors. Unexpected token: I",
     lxb_css_selectors_parse},

    {"[refs=0%]",
     "",
     "Syntax error. Selectors. Unexpected token: 0%",
     lxb_css_selectors_parse},

    {"[refs i]",
     "",
     "Syntax error. Selectors. Unexpected token: i",
     lxb_css_selectors_parse},

    {"['refs']",
     "",
     "Syntax error. Selectors. Unexpected token: \"refs\"",
     lxb_css_selectors_parse},

    {"div #hash [refs=abc]",
     "div #hash [refs=\"abc\"]",
     "",
     lxb_css_selectors_parse},

    {":not()",
     "",
     "Syntax error. Selectors. Pseudo function can't be empty: not()\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {":not(div)",
     ":not(div)",
     "",
     lxb_css_selectors_parse},

    {":NoT(div)",
     ":not(div)",
     "",
     lxb_css_selectors_parse},

    {":not(div, #hash, .class)",
     ":not(div, #hash, .class)",
     "",
     lxb_css_selectors_parse},

    {":not( div,#hash,.class )",
     ":not(div, #hash, .class)",
     "",
     lxb_css_selectors_parse},

    {":not(div, .class 1%)",
     "",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":not(.class 1%, div)",
     "",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":not(div, .class 1%, #hash)",
     "",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":not(div, :not(1%), span)",
     "",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":not(div, :not(), span)",
     "",
     "Syntax error. Selectors. Pseudo function can't be empty: not()\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {":not(div, :not(:not(:not(:not(:not(:not(:not(:not([x])))))))), span)",
     ":not(div, :not(:not(:not(:not(:not(:not(:not(:not([x])))))))), span)",
     "",
     lxb_css_selectors_parse},

    {":not(div, :not(.class, :not([x]), #hash), span)",
     ":not(div, :not(.class, :not([x]), #hash), span)",
     "",
     lxb_css_selectors_parse},

    {":not(div, :not(.class, :not([x], #hash), span)",
     "",
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":not(div, :not(div",
     "",
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":not(div, :not(",
     "",
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":not(div, :not(",
     "",
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":has(div,, .class)",
     ":has(div, .class)",
     "Syntax error. Selectors. Empty Selector List in pseudo function",
     lxb_css_selectors_parse},

    {":has(,div,, .class,)",
     ":has(div, .class)",
     "Syntax error. Selectors. Empty Selector List in pseudo function\n"
     "Syntax error. Selectors. Empty Selector List in pseudo function\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {":has(div, :not(1%), .class)",
     ":has(div, .class)",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":has(div, .class 1%)",
     ":has(div)",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":has(div, .class 1%, #hash)",
     ":has(div, #hash)",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":godofwar(div)",
     "",
     "Syntax error. Selectors. Unexpected token: godofwar(",
     lxb_css_selectors_parse},

    {":has(div, :godofwar(div), .class)",
     ":has(div, .class)",
     "Syntax error. Selectors. Unexpected token: godofwar(",
     lxb_css_selectors_parse},

    {":has(div, :not(1% {la}, (be), [], :fun(x)), .class)",
     ":has(div, .class)",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":has(div, :has(1% {la}, (be), [], :fun(x)), .class)",
     ":has(div, .class)",
     "Syntax error. Selectors. Unexpected token: 1%\n"
     "Syntax error. Selectors. Unexpected token: (\n"
     "Syntax error. Selectors. Unexpected token: ]\n"
     "Syntax error. Selectors. Unexpected token: fun(\n"
     "Syntax error. Selectors. Pseudo function can't be empty: has()\n"
     "Syntax error. Selectors. Empty Selector List in pseudo function",
     lxb_css_selectors_parse},

    {":has(div, :godofwar(",
     ":has(div)",
     "Syntax error. Selectors. Unexpected token: godofwar(\n"
     "Syntax error. Selectors. End Of File in pseudo function",
     lxb_css_selectors_parse},

    {":has(div, :not(div",
     ":has(div)",
     "Syntax error. Selectors. End Of File in pseudo function",
     lxb_css_selectors_parse},

    {":has(div, :not(",
     ":has(div)",
     "Syntax error. Selectors. End Of File in pseudo function",
     lxb_css_selectors_parse},

    {":has(div, :not(:not(",
     ":has(div)",
     "Syntax error. Selectors. End Of File in pseudo function",
     lxb_css_selectors_parse},

    {":has(:not(:not(",
     "",
     "Syntax error. Selectors. End Of File in pseudo function\n"
     "Syntax error. Selectors. Pseudo function can't be empty: has()\n"
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":has(div, :godofwar()))",
     "",
     "Syntax error. Selectors. Unexpected token: godofwar(\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {":has(div, :godofwar(:godofwar()))",
     ":has(div)",
     "Syntax error. Selectors. Unexpected token: godofwar(",
     lxb_css_selectors_parse},

    {":has(div, :godofwar(:godofwar())))",
     "",
     "Syntax error. Selectors. Unexpected token: godofwar(\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {"div, [refs='link'], #hash",
     "div, [refs=\"link\"], #hash",
     "",
     lxb_css_selectors_parse},

    {"div, .class 1%, #hash",
     "",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {"div, .class >, #hash",
     "",
     "Syntax error. Selectors. Unexpected token: ,",
     lxb_css_selectors_parse},

    {"> .class",
     "",
     "Syntax error. Selectors. Unexpected token: >",
     lxb_css_selectors_parse},

    {".class >",
     "",
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {", .class",
     "",
     "Syntax error. Selectors. Unexpected token: ,",
     lxb_css_selectors_parse},

    {".class ,",
     "",
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {"div, > .class, #hash",
     "",
     "Syntax error. Selectors. Unexpected token: >",
     lxb_css_selectors_parse},

    {"div > .class + #hash ~ [refs=a].super || #id",
     "div > .class + #hash ~ [refs=\"a\"].super || #id",
     "",
     lxb_css_selectors_parse},

    {"div:disabled",
     "div:disabled",
     "",
     lxb_css_selectors_parse},

    {":godofwar",
     "",
     "Syntax error. Selectors. Unexpected token: godofwar",
     lxb_css_selectors_parse},

    {":has(:disabled)",
     ":has(:disabled)",
     "",
     lxb_css_selectors_parse},

    {":has(:godofwar)",
     "",
     "Syntax error. Selectors. Unexpected token: godofwar\n"
     "Syntax error. Selectors. Pseudo function can't be empty: has()\n"
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {"::godofwar",
     "",
     "Syntax error. Selectors. Unexpected token: godofwar",
     lxb_css_selectors_parse},

    {":has(::godofwar)",
     "",
     "Syntax error. Selectors. Unexpected token: godofwar\n"
     "Syntax error. Selectors. Pseudo function can't be empty: has()\n"
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":has(div, ::godofwar)",
     ":has(div)",
     "Syntax error. Selectors. Unexpected token: godofwar",
     lxb_css_selectors_parse},

    {":has(div, ::godofwar, .class)",
     ":has(div, .class)",
     "Syntax error. Selectors. Unexpected token: godofwar",
     lxb_css_selectors_parse},

    {"",
     "",
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    /* Relative List. */

    {"> div.class",
     "> div.class",
     "",
     lxb_css_selectors_parse_relative_list},

    {"+ div.class",
     "+ div.class",
     "",
     lxb_css_selectors_parse_relative_list},

    {"~ div.class",
     "~ div.class",
     "",
     lxb_css_selectors_parse_relative_list},

    {"|| div.class",
     "|| div.class",
     "",
     lxb_css_selectors_parse_relative_list},

    {"> div > .class + #hash ~ [refs=a].super || #id",
     "> div > .class + #hash ~ [refs=\"a\"].super || #id",
     "",
     lxb_css_selectors_parse_relative_list},

    /* An+B */

    {":nth-child(2n+2)",
     ":nth-child(2n+2)",
     "",
     lxb_css_selectors_parse},

    {":nth-child(2n+)",
     "",
     "Syntax error. Selectors. Pseudo function can't be empty: nth-child()\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {":has(:nth-child(2n+))",
     "",
     "Syntax error. Selectors. Pseudo function can't be empty: nth-child()\n"
     "Syntax error. Selectors. Unexpected token: )\n"
     "Syntax error. Selectors. Pseudo function can't be empty: has()\n"
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":nth-child(2n+",
     "",
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":has(div, :nth-child(2n+))",
     ":has(div)",
     "Syntax error. Selectors. Pseudo function can't be empty: nth-child()\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {":has(div, :nth-child(2n+",
     ":has(div)",
     "Syntax error. Selectors. End Of File in pseudo function",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of div)",
     ":nth-child(2n+2 of div)",
     "",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of div > .class + #hash ~ [refs=a].super || #id)",
     ":nth-child(2n+2 of div > .class + #hash ~ [refs=\"a\"].super || #id)",
     "",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of )",
     "",
     "Syntax error. Selectors. Pseudo function can't be empty: nth-child()\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of 1%)",
     "",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of :has(div, 1%))",
     ":nth-child(2n+2 of :has(div))",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of :nth-child(2n+1))",
     ":nth-child(2n+2 of :nth-child(odd))",
     "",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of :nth-child(2n+1",
     "",
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of :nth-child(1%",
     "",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of :has(, :nth-child(1%)))",
     "",
     "Syntax error. Selectors. Empty Selector List in pseudo function\n"
     "Syntax error. Selectors. Unexpected token: 1%\n"
     "Syntax error. Selectors. Pseudo function can't be empty: has()\n"
     "Syntax error. Selectors. Pseudo function can't be empty: nth-child()\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {":nth-child(2n+2 of :has(, :nth-child(1%),))",
     "",
     "Syntax error. Selectors. Empty Selector List in pseudo function\n"
     "Syntax error. Selectors. Unexpected token: 1%\n"
     "Syntax error. Selectors. Unexpected token: )\n"
     "Syntax error. Selectors. Pseudo function can't be empty: has()\n"
     "Syntax error. Selectors. Pseudo function can't be empty: nth-child()\n"
     "Syntax error. Selectors. Unexpected token: )",
     lxb_css_selectors_parse},

    {":nth-child(1%)",
     "",
     "Syntax error. Selectors. Unexpected token: 1%",
     lxb_css_selectors_parse},

    /* Block. */

    {":has(:not(div, {}, .class), #hash)",
     ":has(#hash)",
     "Syntax error. Selectors. Unexpected token: {",
     lxb_css_selectors_parse},

    {":has(:not(div, {, .class), #hash)",
     "",
     "Syntax error. Selectors. Unexpected token: {\n"
     "Syntax error. Selectors. End Of File in pseudo function\n"
     "Syntax error. Selectors. Pseudo function can't be empty: has()\n"
     "Syntax error. Selectors. Unexpected token: END-OF-FILE",
     lxb_css_selectors_parse},

    {":has(:not(div, {([([{}])])}, .class), #hash)",
     ":has(#hash)",
     "Syntax error. Selectors. Unexpected token: {",
     lxb_css_selectors_parse},

    {":has(:not(div, {([([{, .class}])])}), #hash)",
     ":has(#hash)",
     "Syntax error. Selectors. Unexpected token: {",
     lxb_css_selectors_parse},

    {"div > :nth-child(2n+1):not(:has(a)),p:has(a):not([href]) span, div",
     "div > :nth-child(odd):not(:has(a)), p:has(a):not([href]) span, div",
     "",
     lxb_css_selectors_parse},

    {"div > :nth-child(2n+1) :not(:has(a)),p:has(a) :not([href]) span, div",
     "div > :nth-child(odd) :not(:has(a)), p:has(a) :not([href]) span, div",
     "",
     lxb_css_selectors_parse},
};


TEST_BEGIN(lexbor_selectors_list)
{
    char *test, *have, *need,*erro, *log;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_selector_list_t *list;

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

        list = selectors_list[i].handler(parser, (lxb_char_t *)
                                         test, strlen(test));

        have = (char *) lxb_css_selector_serialize_list_chain_char(list, NULL);
        test_eq_str(have, need);

        if (have != NULL) {
            lexbor_free(have);
        }

        log = (char *) lxb_css_log_serialize_char(parser->log, NULL, NULL, 0);
        test_eq_str(log, erro);

        if (log != NULL) {
            lexbor_free(log);
        }

        lxb_css_selector_list_destroy_memory(list);

        TEST_PRINT("ok\n");
    }

    lxb_css_parser_destroy(parser, true);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(lexbor_selectors_list);

    TEST_RUN("lexbor/css/selectors/selectors");
    TEST_RELEASE();
}
