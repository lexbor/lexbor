/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>

#include <unit/test.h>


typedef struct {
    lexbor_str_t selector;
    uint32_t     a;
    uint32_t     b;
    uint32_t     c;
}
test_data_t;

static const test_data_t lxb_data[] = {
    {
        .selector = lexbor_str("#id .class tag"),
        .a = 1, .b = 1, .c = 1
    },
    {
        .selector = lexbor_str("#id #id .class tag"),
        .a = 2, .b = 1, .c = 1
    },
    {
        .selector = lexbor_str("#id .class .class tag"),
        .a = 1, .b = 2, .c = 1
    },
    {
        .selector = lexbor_str("#id .class tag tag"),
        .a = 1, .b = 1, .c = 2
    },
    {
        .selector = lexbor_str("#id tag :nth-child(even of #item .class)"),
        .a = 2, .b = 1, .c = 1
    },
    {
        .selector = lexbor_str("#id tag :nth-child(even of #item.class)"),
        .a = 2, .b = 2, .c = 1
    },
    {
        .selector = lexbor_str("#id tag :nth-child(even of #item, .class)"),
        .a = 2, .b = 1, .c = 1
    },
    {
        .selector = lexbor_str("#id :is(#item.class, #id)"),
        .a = 2, .b = 1, .c = 0
    },
    {
        .selector = lexbor_str("#id tag :nth-child(even)"),
        .a = 1, .b = 1, .c = 1
    },
    {
        .selector = lexbor_str("tag#id:nth-child(even)"),
        .a = 1, .b = 1, .c = 1
    },
    {
        .selector = lexbor_str("[a=b]"),
        .a = 0, .b = 1, .c = 0
    },
    {
        .selector = lexbor_str("#id tag :is(#item, .class)"),
        .a = 2, .b = 0, .c = 1
    },
    {
        .selector = lexbor_str("#id tag :not(#item, .class)"),
        .a = 2, .b = 0, .c = 1
    },
    {
        .selector = lexbor_str("#id tag :has(#item, .class)"),
        .a = 2, .b = 0, .c = 1
    },
    {
        .selector = lexbor_str("#id tag :where(#item, .class)"),
        .a = 1, .b = 0, .c = 1
    },
    {
        .selector = lexbor_str("#id:is(#item:is(#new))"),
        .a = 2, .b = 0, .c = 0
    },
    {
        .selector = lexbor_str("*"),
        .a = 0, .b = 0, .c = 0
    },
    {
        .selector = lexbor_str("LI"),
        .a = 0, .b = 0, .c = 1
    },
    {
        .selector = lexbor_str("UL LI"),
        .a = 0, .b = 0, .c = 2
    },
    {
        .selector = lexbor_str("UL OL+LI"),
        .a = 0, .b = 0, .c = 3
    },
    {
        .selector = lexbor_str("H1 + *[REL=up]"),
        .a = 0, .b = 1, .c = 1
    },
    {
        .selector = lexbor_str("UL OL LI.red"),
        .a = 0, .b = 1, .c = 3
    },
    {
        .selector = lexbor_str("LI.red.level"),
        .a = 0, .b = 2, .c = 1
    },
    {
        .selector = lexbor_str("#x34y"),
        .a = 1, .b = 0, .c = 0
    },
    {
        .selector = lexbor_str("#s12:not(FOO)"),
        .a = 1, .b = 0, .c = 1
    },
    {
        .selector = lexbor_str(".foo :is(.bar, #baz)"),
        .a = 1, .b = 1, .c = 0
    },
};

static const int lxb_data_length = sizeof(lxb_data) / sizeof(test_data_t);


TEST_BEGIN(lexbor_specificity)
{
    int i;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_memory_t *memory;
    lxb_css_selector_list_t *list;
    const test_data_t *data;

    memory = lxb_css_memory_create();
    status = lxb_css_memory_init(memory, 128);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    lxb_css_parser_memory_set(parser, memory);

    for (i = 0; i < lxb_data_length; i++) {
        data = &lxb_data[i];

        TEST_PRINTLN("%d) %s", i + 1, data->selector.data);

        list = lxb_css_selectors_parse(parser, data->selector.data,
                                       data->selector.length);
        test_ne(list, NULL);

        test_eq(lxb_css_selector_sp_a(list->specificity), data->a);
        test_eq(lxb_css_selector_sp_b(list->specificity), data->b);
        test_eq(lxb_css_selector_sp_c(list->specificity), data->c);
    }

    (void) lxb_css_memory_destroy(memory, true);
    (void) lxb_css_parser_destroy(parser, true);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(lexbor_specificity);

    TEST_RUN("lexbor/selectors/specificity");
    TEST_RELEASE();
}
