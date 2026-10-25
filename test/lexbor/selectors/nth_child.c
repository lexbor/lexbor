/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "helper.h"


static const char compound_html[] =
    "<!DOCTYPE html><div id=r><span id=s class=a></span>"
    "text<!--comment--><p id=p class=a></p></div>";

static const char compound_reverse_html[] =
    "<!DOCTYPE html><div id=r><p id=p class=a></p>"
    "text<!--comment--><span id=s class=a></span></div>";

static const char list_html[] =
    "<!DOCTYPE html><div id=r><p id=p class=a></p>"
    "text<!--comment--><span id=s class=b></span>"
    "<p id=q class=a></p></div>";

static const char complex_html[] =
    "<!DOCTYPE html><div class=x><div class=p><div class=q><div class=q>"
    "<b id=b class=z></b>text<!--comment--><span id=s class=z></span>"
    "<i id=t class=z></i></div></div></div></div>";


TEST_BEGIN(compound)
{
    static const test_selector_case_t cases[] = {
        {compound_html, "p:nth-child(1 of p.a)", {{"p", 0}}},
        {compound_html, "p:nth-child(2 of p.a)", {{NULL, 0}}},
        {compound_reverse_html, "p:nth-last-child(1 of p.a)", {{"p", 0}}},
        {compound_reverse_html, "p:nth-last-child(2 of p.a)", {{NULL, 0}}}
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(test_selector_case_t); i++) {
        TEST_CALL_ARGS(test_selector_case, &cases[i]);
    }
}
TEST_END

TEST_BEGIN(list)
{
    static const test_selector_case_t cases[] = {
        {list_html, ":nth-child(1 of p.a, span.b)", {{"p", 0}}},
        {list_html, ":nth-child(2 of p.a, span.b)", {{"s", 0}}},
        {list_html, ":nth-child(3 of p.a, span.b)", {{"q", 0}}},
        {list_html, ":nth-last-child(1 of p.a, span.b)", {{"q", 0}}},
        {list_html, ":nth-last-child(2 of p.a, span.b)", {{"s", 0}}},
        {list_html, ":nth-last-child(3 of p.a, span.b)", {{"p", 0}}},
        {list_html, ":nth-child(2n+1 of p.a, span.b)", {{"p", 0}, {"q", 0}}},
        {list_html, ":nth-child(2 of .a, p.a)", {{"q", 0}}},
        {list_html, ":nth-last-child(2 of .a, p.a)", {{"p", 0}}},
        {list_html, ":nth-child(3 of .missing .a, p.a, span.b)", {{"q", 0}}},
        {list_html, ":nth-last-child(3 of .missing .a, p.a, span.b)",
         {{"p", 0}}},
        {list_html, ":nth-child(n of p.a, span.b), #s",
         {{"p", 0}, {"s", 0}, {"s", 1}, {"q", 0}}},
        {list_html, ".missing .a, :nth-child(3 of p.a, span.b)", {{"q", 1}}}
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(test_selector_case_t); i++) {
        TEST_CALL_ARGS(test_selector_case, &cases[i]);
    }
}
TEST_END

TEST_BEGIN(complex)
{
    static const test_selector_case_t cases[] = {
        {complex_html, ":nth-child(1 of .x > .p .q .z)", {{"b", 0}}},
        {complex_html, ":nth-child(2 of .x > .p .q .z)", {{"s", 0}}},
        {complex_html, ":nth-child(3 of .x > .p .q .z)", {{"t", 0}}},
        {complex_html, ":nth-child(2n+1 of .x > .p .q .z)",
         {{"b", 0}, {"t", 0}}},
        {complex_html, ":nth-child(-n+2 of .x > .p .q .z)",
         {{"b", 0}, {"s", 0}}},
        {complex_html, ":nth-child(n of .x > .p .q .z)",
         {{"b", 0}, {"s", 0}, {"t", 0}}},
        {complex_html, ":nth-last-child(1 of .x > .p .q .z)", {{"t", 0}}},
        {complex_html, ":nth-last-child(2 of .x > .p .q .z)", {{"s", 0}}},
        {complex_html, ":nth-last-child(3 of .x > .p .q .z)", {{"b", 0}}},
        {complex_html, ":nth-last-child(2n+1 of .x > .p .q .z)",
         {{"b", 0}, {"t", 0}}},
        {complex_html, ":nth-last-child(-n+2 of .x > .p .q .z)",
         {{"s", 0}, {"t", 0}}},
        {complex_html, ":nth-last-child(n of .x > .p .q .z)",
         {{"b", 0}, {"s", 0}, {"t", 0}}}
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(test_selector_case_t); i++) {
        TEST_CALL_ARGS(test_selector_case, &cases[i]);
    }
}
TEST_END

TEST_BEGIN(nested)
{
    static const test_selector_case_t cases[] = {
        {list_html, ":is(:nth-child(3 of p.a, span.b))", {{"q", 0}}},
        {list_html, ":where(:nth-last-child(3 of p.a, span.b))", {{"p", 0}}},
        {list_html, ":nth-child(3 of p:has(+ span), span.b, p.a)", {{"q", 0}}},
        {list_html, "p:not(:nth-child(3 of p.a, span.b))", {{"p", 0}}},
        {list_html, ":nth-child(3 of p:is(.a), span:where(.b))", {{"q", 0}}},
        {list_html, ":nth-last-child(3 of p:is(.a), span:where(.b))",
         {{"p", 0}}},
        {list_html, ":nth-child(3 of :is(p.a, span.b))", {{"q", 0}}},
        {list_html, ":nth-child(2 of :nth-child(odd))", {{"q", 0}}},
        {list_html, ":nth-last-child(2 of :nth-last-child(odd))", {{"p", 0}}}
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(test_selector_case_t); i++) {
        TEST_CALL_ARGS(test_selector_case, &cases[i]);
    }
}
TEST_END

int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(compound);
    TEST_ADD(list);
    TEST_ADD(complex);
    TEST_ADD(nested);

    TEST_RUN("lexbor/selectors/nth_child");
    TEST_RELEASE();
}
