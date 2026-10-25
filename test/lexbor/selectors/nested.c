/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "helper.h"


static const test_selector_case_t cases[] = {
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":is(.x) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":where(.x) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=y><p id=near class=x><span id=t></span></p></div>"
        "</body></html>",
        ":not(.x) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far><i id=mark class=mark></i><p id=near><span id=t></span>"
        "</p></div></body></html>",
        ":has(> .mark) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":nth-child(1 of .x) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":nth-last-child(1 of .x) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><i id=prefix class=x></i><p id=near class=x>"
        "<span id=t></span></p></div></body></html>",
        ":nth-child(1 of .x) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near class=x><span id=t></span></p>"
        "<i id=suffix class=x></i></div></body></html>",
        ":nth-last-child(1 of .x) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":is(:where(.x)) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=\"x enabled\"><p id=near class=enabled><span id=t>"
        "</span></p></div></body></html>",
        ":is(.x).enabled #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><div id=near><span id=t></span></div></div>"
        "</body></html>",
        "div:is(.x) #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":is(.absent) #t",
        {{NULL, 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":is(.absent) #t, #t",
        {{"t", 1}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":is(.x) #t, #t",
        {{"t", 0}, {"t", 1}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<i id=far class=x></i>text<!--between--><b id=near></b><span id=t>"
        "</span></body></html>",
        ":is(.x) ~ #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<i id=far class=x></i>text<!--between--><b id=near></b><span id=t>"
        "</span></body></html>",
        ":where(.x) ~ #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<i id=far class=y></i><b id=near class=x></b><span id=t></span>"
        "</body></html>",
        ":not(.x) ~ #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far><i id=mark class=mark></i></div><b id=near></b>"
        "<span id=t></span></body></html>",
        ":has(> .mark) ~ #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<i id=far class=x></i>text<!--between--><b id=near></b><span id=t>"
        "</span></body></html>",
        ":nth-child(1 of .x) ~ #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<i id=far class=x></i>text<!--between--><b id=near></b><span id=t>"
        "</span></body></html>",
        ":nth-last-child(1 of .x) ~ #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<i id=far class=x></i>text<!--between--><b id=near class=x></b>"
        "<span id=t></span></body></html>",
        ":nth-child(1 of .x) ~ #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<i id=far class=x></i>text<!--between--><b id=near class=x></b>"
        "<span id=t></span></body></html>",
        ":nth-last-child(2 of .x) ~ #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":is(.x) > #t",
        {{NULL, 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far><p id=near class=x><span id=t></span></p></div></body>"
        "</html>",
        ":is(.x) > #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<i id=far class=x></i>text<!--between--><b id=near></b><span id=t>"
        "</span></body></html>",
        ":is(.x) + #t",
        {{NULL, 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<i id=far></i><b id=near class=x></b><span id=t></span></body>"
        "</html>",
        ":is(.x) + #t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        ":is(.x)#t",
        {{NULL, 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=far class=x><p id=near><span id=t></span></p></div></body>"
        "</html>",
        "#t:is(.x)",
        {{NULL, 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<span id=t class=x></span></body></html>",
        ":is(.x)#t",
        {{"t", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(:is(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(:where(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near class=x></p><section id=far class=y></section>"
        "</div></body></html>",
        "#r:has(:not(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(:nth-child(1 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(:nth-last-child(1 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near class=x></p>text<!--between-->"
        "<section id=far class=x></section></div></body></html>",
        "#r:has(:nth-child(2 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near class=x></p>text<!--between-->"
        "<section id=far class=x></section></div></body></html>",
        "#r:has(:nth-last-child(1 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(> :is(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(> :where(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near class=x></p><section id=far class=y></section>"
        "</div></body></html>",
        "#r:has(> :not(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(> :nth-child(1 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(> :nth-last-child(1 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near class=x></p>text<!--between-->"
        "<section id=far class=x></section></div></body></html>",
        "#r:has(> :nth-child(2 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near class=x></p>text<!--between-->"
        "<section id=far class=x></section></div></body></html>",
        "#r:has(> :nth-last-child(1 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(:is(:where(.x)))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near class=enabled></p>"
        "<section id=far class=\"x enabled\"></section></div></body></html>",
        "#r:has(:is(.x).enabled)",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><section id=near></section><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(section:is(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(:is(.absent))",
        {{NULL, 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(:is(.absent)), #r",
        {{"r", 1}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near></p>text<!--between--><section id=far class=x>"
        "</section></div></body></html>",
        "#r:has(:is(.x)), #r",
        {{"r", 0}, {"r", 1}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near><i id=deep class=x></i></p><section id=far>"
        "</section></div></body></html>",
        "#r:has(> :is(.x))",
        {{NULL, 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r><p id=near><i id=deep class=x></i></p>"
        "<section id=far class=x></section></div></body></html>",
        "#r:has(> :is(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div>text<!--between--><b id=near></b><i id=far class=x>"
        "</i></body></html>",
        "#r:has(~ :is(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div>text<!--between--><b id=near></b><i id=far class=x>"
        "</i></body></html>",
        "#r:has(~ :where(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div><b id=near class=x></b><i id=far class=y></i></body>"
        "</html>",
        "#r:has(~ :not(.x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div>text<!--between--><b id=near></b><i id=far class=x>"
        "</i></body></html>",
        "#r:has(~ :nth-child(1 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div>text<!--between--><b id=near></b><i id=far class=x>"
        "</i></body></html>",
        "#r:has(~ :nth-last-child(1 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div><b id=near class=x></b><i id=far class=x></i></body>"
        "</html>",
        "#r:has(~ :nth-child(2 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div><b id=near class=x></b><i id=far class=x></i></body>"
        "</html>",
        "#r:has(~ :nth-last-child(1 of .x))",
        {{"r", 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div></body></html>",
        "#r:has(~ :is(.x))",
        {{NULL, 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div>text<!--between--><b id=near></b><i id=far class=x>"
        "</i></body></html>",
        "#r:has(+ :is(.x))",
        {{NULL, 0}}
    },
    {
        "<!doctype html><html id=html><head id=head></head><body id=body>"
        "<div id=r></div><b id=near class=x></b><i id=far></i></body></html>",
        "#r:has(+ :is(.x))",
        {{"r", 0}}
    },
    {
        "<div id=r><p id=p class=a></p>text<!--comment--><span id=s class=b>"
        "</span><p id=q class=a></p></div>",
        "div:has(> :nth-child(3 of p.a, span.b))",
        {{"r", 0}}
    },
    {
        "<div id=outer class=y><i id=a class=c><span id=s class=c></span></i>"
        "<i id=b class=y></i></div>",
        ":nth-last-child(2n+1 of .y, .c) span",
        {{"s", 0}}
    },
    {
        "<div><article class=y></article><i><span id=x></span>"
        "<article class=y><span id=s></span></article></i></div>",
        ":nth-child(2 of i, .y:not(.missing)) span",
        {{"x", 0}, {"s", 0}}
    }
};


TEST_BEGIN(nested_rejected_candidate)
{
    size_t length = sizeof(cases) / sizeof(test_selector_case_t);

    for (size_t i = 0; i < length; i++) {
        TEST_CALL_ARGS(test_selector_case, &cases[i]);
    }
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(nested_rejected_candidate);

    TEST_RUN("lexbor/selectors/nested");
    TEST_RELEASE();
}
