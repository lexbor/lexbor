/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/css/css.h>
#include <lexbor/selectors/selectors.h>

#include <unit/test.h>


typedef struct {
    char *test;
    char *result;
}
lxb_test_entry_t;

static const lxb_char_t html[] =
    "<div div='First' class='Strong Massive'>"
    "    <p p=1><a a=1>a1</a></p>"
    "    <p p=2><a a=2>a2</a></p>"
    "    <p p=3><a a=3>a3</a></p>"
    "    <p p=4><a a=4>a4</a></p>"
    "    <p p=5><a a=5>a5</a></p>"
    "</div>"
    "<div div='Second' class='Massive Stupid'>"
    "<p p=6 lang='en-GB'>"
    "    <span id=s1 span=1></span>"
    "    <span id=s2 span=2></span>"
    "    <span id=s3 span=3></span>"
    "    <span id=s4 span=4></span>"
    "    <span id=s5 span=5></span>"
    "</p>"
    "<p p=7 lang='ru'>"
    "    <span span=6></span>"
    "    <a a=6><span span=7></span></a>"
    "    <a a=7><span span=8></span></a>"
    "    <span span=9></span>"
    "    <a a=8></a>"
    "    <span span=10></span>"
    "    <span test span=11></span>"
    "    <span test='' span=12></span>"
    "</p>"
    "</div>";


static const lxb_test_entry_t selectors_list[] =
{
    {"a",
     "<a a=\"1\">\n"
     "<a a=\"2\">\n"
     "<a a=\"3\">\n"
     "<a a=\"4\">\n"
     "<a a=\"5\">\n"
     "<a a=\"6\">\n"
     "<a a=\"7\">\n"
     "<a a=\"8\">"},

    {"#s3",
     "<span id=\"s3\" span=\"3\">"},

    {"[p]",
     "<p p=\"1\">\n"
     "<p p=\"2\">\n"
     "<p p=\"3\">\n"
     "<p p=\"4\">\n"
     "<p p=\"5\">\n"
     "<p p=\"6\" lang=\"en-GB\">\n"
     "<p p=\"7\" lang=\"ru\">"},

    {"[p = '2']",
     "<p p=\"2\">"},

    {"[div = 'first']",
     ""},

    {"[div = 'first' i]",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[test = '']",
     "<span test span=\"11\">\n"
     "<span test=\"\" span=\"12\">"},

    {"[class ~= 'massive']",
     ""},

    {"[class ~= 'Massive']",
     "<div div=\"First\" class=\"Strong Massive\">\n"
     "<div div=\"Second\" class=\"Massive Stupid\">"},

    {"[class ~= 'massive' i]",
     "<div div=\"First\" class=\"Strong Massive\">\n"
     "<div div=\"Second\" class=\"Massive Stupid\">"},

    {"[test ~= '']",
     ""},

    {"[lang |= 'en']",
     "<p p=\"6\" lang=\"en-GB\">"},

    {"[lang |= 'eN']",
     ""},

    {"[lang |= 'eN' i]",
     "<p p=\"6\" lang=\"en-GB\">"},

    {"[lang |= 'ru']",
     "<p p=\"7\" lang=\"ru\">"},

    {"[lang |= 'r']",
     ""},

    {"[test |= '']",
     "<span test span=\"11\">\n"
     "<span test=\"\" span=\"12\">"},

    {"[div ^= 'Fir']",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[div ^= 'fir']",
     ""},

    {"[div ^= 'fir' i]",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[div ^= 'irst']",
     ""},

    {"[div ^= 'First']",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[test ^= '']",
     ""},

    {"[div $= 'irst']",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[div $= 't']",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[div $= 'First']",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[div $= 'rSt']",
     ""},

    {"[div $= 'rSt' i]",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[div $= 'Firs']",
     ""},

    {"[test $= '']",
     ""},

    {"[div *= 'irs']",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[div *= 'iRs']",
     ""},

    {"[div *= 'iRs' i]",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[div *= 'First']",
     "<div div=\"First\" class=\"Strong Massive\">"},

    {"[div *= '']",
     ""},

    {"[test *= '']",
     ""},

    {"*",
     "<div div=\"First\" class=\"Strong Massive\">\n"
     "<p p=\"1\">\n"
     "<a a=\"1\">\n"
     "<p p=\"2\">\n"
     "<a a=\"2\">\n"
     "<p p=\"3\">\n"
     "<a a=\"3\">\n"
     "<p p=\"4\">\n"
     "<a a=\"4\">\n"
     "<p p=\"5\">\n"
     "<a a=\"5\">\n"
     "<div div=\"Second\" class=\"Massive Stupid\">\n"
     "<p p=\"6\" lang=\"en-GB\">\n"
     "<span id=\"s1\" span=\"1\">\n"
     "<span id=\"s2\" span=\"2\">\n"
     "<span id=\"s3\" span=\"3\">\n"
     "<span id=\"s4\" span=\"4\">\n"
     "<span id=\"s5\" span=\"5\">\n"
     "<p p=\"7\" lang=\"ru\">\n"
     "<span span=\"6\">\n"
     "<a a=\"6\">\n"
     "<span span=\"7\">\n"
     "<a a=\"7\">\n"
     "<span span=\"8\">\n"
     "<span span=\"9\">\n"
     "<a a=\"8\">\n"
     "<span span=\"10\">\n"
     "<span test span=\"11\">\n"
     "<span test=\"\" span=\"12\">"},

    {"div span",
     "<span id=\"s1\" span=\"1\">\n"
     "<span id=\"s2\" span=\"2\">\n"
     "<span id=\"s3\" span=\"3\">\n"
     "<span id=\"s4\" span=\"4\">\n"
     "<span id=\"s5\" span=\"5\">\n"
     "<span span=\"6\">\n"
     "<span span=\"7\">\n"
     "<span span=\"8\">\n"
     "<span span=\"9\">\n"
     "<span span=\"10\">\n"
     "<span test span=\"11\">\n"
     "<span test=\"\" span=\"12\">"},

    {"div > span",
     ""},

    {"p > span",
     "<span id=\"s1\" span=\"1\">\n"
     "<span id=\"s2\" span=\"2\">\n"
     "<span id=\"s3\" span=\"3\">\n"
     "<span id=\"s4\" span=\"4\">\n"
     "<span id=\"s5\" span=\"5\">\n"
     "<span span=\"6\">\n"
     "<span span=\"9\">\n"
     "<span span=\"10\">\n"
     "<span test span=\"11\">\n"
     "<span test=\"\" span=\"12\">"},

    {"div > p > a",
     "<a a=\"1\">\n"
     "<a a=\"2\">\n"
     "<a a=\"3\">\n"
     "<a a=\"4\">\n"
     "<a a=\"5\">\n"
     "<a a=\"6\">\n"
     "<a a=\"7\">\n"
     "<a a=\"8\">"},

    {"span ~ span",
     "<span id=\"s2\" span=\"2\">\n"
     "<span id=\"s3\" span=\"3\">\n"
     "<span id=\"s4\" span=\"4\">\n"
     "<span id=\"s5\" span=\"5\">\n"
     "<span id=\"s3\" span=\"3\">\n"
     "<span id=\"s4\" span=\"4\">\n"
     "<span id=\"s5\" span=\"5\">\n"
     "<span id=\"s4\" span=\"4\">\n"
     "<span id=\"s5\" span=\"5\">\n"
     "<span id=\"s5\" span=\"5\">\n"
     "<span span=\"9\">\n"
     "<span span=\"10\">\n"
     "<span test span=\"11\">\n"
     "<span test=\"\" span=\"12\">\n"
     "<span span=\"10\">\n"
     "<span test span=\"11\">\n"
     "<span test=\"\" span=\"12\">\n"
     "<span test span=\"11\">\n"
     "<span test=\"\" span=\"12\">\n"
     "<span test=\"\" span=\"12\">"},

    {"p[p='2'] + p",
     "<p p=\"3\">"},

    {"p[p='2'] + p[p='4']",
     ""},

    {"p:has(a)",
     "<p p=\"1\">\n"
     "<p p=\"2\">\n"
     "<p p=\"3\">\n"
     "<p p=\"4\">\n"
     "<p p=\"5\">\n"
     "<p p=\"7\" lang=\"ru\">"},

    {"p:is([p='2'], [p='5'])",
     "<p p=\"2\">\n"
     "<p p=\"5\">"},

    {"div:has(p :not(span))",
     "<div div=\"First\" class=\"Strong Massive\">\n"
     "<div div=\"Second\" class=\"Massive Stupid\">"},

    {":not(span, div)",
     "<p p=\"1\">\n"
     "<a a=\"1\">\n"
     "<p p=\"2\">\n"
     "<a a=\"2\">\n"
     "<p p=\"3\">\n"
     "<a a=\"3\">\n"
     "<p p=\"4\">\n"
     "<a a=\"4\">\n"
     "<p p=\"5\">\n"
     "<a a=\"5\">\n"
     "<p p=\"6\" lang=\"en-GB\">\n"
     "<p p=\"7\" lang=\"ru\">\n"
     "<a a=\"6\">\n"
     "<a a=\"7\">\n"
     "<a a=\"8\">"},

    {"p[p='7'] span:nth-child(2n+1)",
     "<span span=\"6\">\n"
     "<span span=\"7\">\n"
     "<span span=\"8\">\n"
     "<span test span=\"11\">"},

    {"p[p='7'] > span:nth-child(2n+1)",
     "<span span=\"6\">\n"
     "<span test span=\"11\">"},

    {"p[p='7'] > span:nth-child(2n+1 of [test])",
     "<span test span=\"11\">"},

    {"p[p='7'] span:nth-last-child(2n+1)",
     "<span span=\"7\">\n"
     "<span span=\"8\">\n"
     "<span span=\"9\">\n"
     "<span span=\"10\">\n"
     "<span test=\"\" span=\"12\">"},

    {"p[p='7'] > span:nth-last-child(2n+1 of [test])",
     "<span test=\"\" span=\"12\">"},

    {"p[p='7'] > span:nth-of-type(2n+1)",
     "<span span=\"6\">\n"
     "<span span=\"10\">\n"
     "<span test=\"\" span=\"12\">"},

    {"p[p='7'] > span:nth-last-of-type(2n+1)",
     "<span span=\"6\">\n"
     "<span span=\"10\">\n"
     "<span test=\"\" span=\"12\">"},

    {"p :where(span#s4)",
     "<span id=\"s4\" span=\"4\">"},

    {"span[id = 's4']#s4[span = '4']",
     "<span id=\"s4\" span=\"4\">"},
    {"p[p='6'][lang |= 'en'] span[id='s4']#s4[span='4']",
     "<span id=\"s4\" span=\"4\">"},

    {"div > :nth-child(2n+1):not(:has(a))",
     "<p p=\"6\" lang=\"en-GB\">"},

    {"div > :nth-child(2n+1) :not(:has(a))",
     "<a a=\"1\">\n"
     "<a a=\"3\">\n"
     "<a a=\"5\">\n"
     "<span id=\"s1\" span=\"1\">\n"
     "<span id=\"s2\" span=\"2\">\n"
     "<span id=\"s3\" span=\"3\">\n"
     "<span id=\"s4\" span=\"4\">\n"
     "<span id=\"s5\" span=\"5\">"},

    {"div > :has(, a)",
     "<p p=\"1\">\n"
     "<p p=\"2\">\n"
     "<p p=\"3\">\n"
     "<p p=\"4\">\n"
     "<p p=\"5\">\n"
     "<p p=\"7\" lang=\"ru\">"},
};


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lexbor_str_t *str = ctx;

    memcpy(&str->data[str->length], data, len);
    str->length += len;

    str->data[str->length] = '\0';

    return LXB_STATUS_OK;
}

lxb_status_t
find_callback(lxb_dom_node_t *node, lxb_css_selector_specificity_t *spec,
              void *ctx)
{
    lexbor_str_t *str = ctx;

    if (str->length != 0) {
        memcpy(&str->data[str->length], "\n\0", 2);
        str->length++;
    }

    return lxb_html_serialize_cb(node, callback, ctx);
}

TEST_BEGIN(lexbor_selectors_list)
{
    char *test, *need;
    lexbor_str_t str;
    lxb_status_t status;
    lxb_dom_node_t *body;
    lxb_selectors_t *selectors;
    lxb_css_parser_t *parser;
    lxb_css_selector_list_t *list;
    lxb_html_document_t *document;
    lxb_css_selectors_t *css_selectors;

    size_t entries_length = sizeof(selectors_list) / sizeof(lxb_test_entry_t);

    /* Create HTML Document. */

    document = lxb_html_document_create();
    status = lxb_html_document_parse(document, html,
                                     (sizeof(html) / sizeof(lxb_char_t)) - 1);
    test_eq(status, LXB_STATUS_OK);

    body = lxb_dom_interface_node(lxb_html_document_body_element(document));

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL, NULL);
    test_eq(status, LXB_STATUS_OK);

    css_selectors = lxb_css_selectors_create();
    status = lxb_css_selectors_init(css_selectors, 32);
    test_eq(status, LXB_STATUS_OK);

    lxb_css_parser_selectors_set(parser, css_selectors);

    selectors = lxb_selectors_create();
    status = lxb_selectors_init(selectors);
    test_eq(status, LXB_STATUS_OK);

    str.data = lexbor_malloc(4096 * 4);
    str.length = 0;

    TEST_PRINT("\n");

    for (unsigned i = 0; i < entries_length; i++) {
        test = selectors_list[i].test;
        need = selectors_list[i].result;

        TEST_PRINTLN("%d) %s", i + 1, test);

        list = lxb_css_selectors_parse(parser,
                                       (const lxb_char_t *) test, strlen(test));
        test_ne(list, NULL);

        str.data[0] = '\0';
        str.length = 0;

        status = lxb_selectors_find(selectors, body, list,
                                    find_callback, &str);
        test_eq(status, LXB_STATUS_OK);

        test_eq_str(str.data, need);

        lxb_css_selectors_erase(css_selectors);

        TEST_PRINT("ok\n");
    }

    (void) lexbor_free(str.data);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_selectors_destroy(selectors, true);
    (void) lxb_css_selectors_destroy(css_selectors, true, true);
    (void) lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(lexbor_selectors_list);

    TEST_RUN("lexbor/selectors/selectors");
    TEST_RELEASE();
}
