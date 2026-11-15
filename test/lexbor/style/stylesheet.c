/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/style/style.h>
#include <unit/test.h>


TEST_BEGIN_ARGS(style_is, lxb_dom_node_t *node, const lexbor_str_t *need)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_element_t *element;

    out.data = NULL;
    out.length = 0;

    element = lxb_dom_interface_element(node);

    status = lxb_dom_element_style_serialize_str(element, &out,
                                                 LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, need->data, need->length);
}
TEST_END

TEST_BEGIN_ARGS(attach, lxb_html_document_t *document,
                lxb_css_parser_t *parser, const lexbor_str_t *css)
{
    lxb_status_t status;
    lxb_css_stylesheet_t *sst;

    sst = lxb_css_stylesheet_create(NULL);
    status = lxb_css_stylesheet_parse(sst, parser, css->data, css->length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    TEST_RETVAL = sst;
}
TEST_END_RETVAL(TEST_RETVAL)

TEST_BEGIN(two_stylesheet_destroy_all)
{
    lxb_status_t status;
    lxb_html_document_t *document;

    /* HTML Data. */

    static const lexbor_str_t html = lexbor_str("<style>a{color:red}</style>"
                                                "<style>b{color:blue}</style>");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    /* Init all CSS objects and momory for Document. */

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    /* Parse HTML. */

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    /* Destroy resources. */
    (void) lxb_html_document_stylesheet_destroy_all(document, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(destroy_all_own_memory)
{
    lxb_status_t status;
    lxb_dom_node_t *div;
    lxb_css_parser_t *parser;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<div>x</div>");
    static const lexbor_str_t css = lexbor_str("div {color: red}");
    static const lexbor_str_t before = lexbor_str("color: red");
    static const lexbor_str_t after = lexbor_str("");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(attach, document, parser, &css);

    div = lxb_dom_interface_node(document->body)->first_child;

    TEST_CALL_ARGS(style_is, div, &before);

    lxb_html_document_stylesheet_destroy_all(document, true);

    TEST_CALL_ARGS(style_is, div, &after);

    test_eq(lxb_dom_element_style_by_id(lxb_dom_interface_element(div),
                                        LXB_CSS_PROPERTY_COLOR), NULL);

    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(destroy_all_changed_class)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_node_t *div;
    lxb_css_parser_t *parser;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<div class=a>x</div>");
    static const lexbor_str_t css = lexbor_str(".a {color: red}");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t b_str = lexbor_str("b");
    static const lexbor_str_t after = lexbor_str("");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(attach, document, parser, &css);

    div = lxb_dom_interface_node(document->body)->first_child;

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         b_str.data, b_str.length);
    test_ne(attr, NULL);

    lxb_html_document_stylesheet_destroy_all(document, true);

    TEST_CALL_ARGS(style_is, div, &after);

    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(destroy_all_style_element)
{
    lxb_status_t status;
    lxb_dom_node_t *div, *style;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<style>div {color: red}</style>"
                                                "<div>x</div>");

    static const lexbor_str_t before = lexbor_str("color: red");
    static const lexbor_str_t after = lexbor_str("");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    div = lxb_dom_interface_node(document->body)->first_child;
    style = lxb_dom_interface_node(document->head)->first_child;

    TEST_CALL_ARGS(style_is, div, &before);

    lxb_html_document_stylesheet_destroy_all(document, true);

    TEST_CALL_ARGS(style_is, div, &after);

    test_eq(lxb_html_interface_style(style)->stylesheet, NULL);

    lxb_dom_node_remove(style);

    TEST_CALL_ARGS(style_is, div, &after);

    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(destroy_all_attach)
{
    lxb_status_t status;
    lxb_dom_node_t *div;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<div style='width: 10px'>"
                                                "x</div>");

    static const lexbor_str_t red = lexbor_str("div {color: red}");
    static const lexbor_str_t blue = lexbor_str("div {color: blue}");
    static const lexbor_str_t before = lexbor_str("color: red; width: 10px");
    static const lexbor_str_t inline_str = lexbor_str("width: 10px");
    static const lexbor_str_t after = lexbor_str("color: blue; width: 10px");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(attach, document, parser, &red);

    div = lxb_dom_interface_node(document->body)->first_child;

    TEST_CALL_ARGS(style_is, div, &before);

    lxb_html_document_stylesheet_destroy_all(document, true);

    TEST_CALL_ARGS(style_is, div, &inline_str);

    TEST_CALL_ARGS(attach, document, parser, &blue);
    sst = TEST_RETVAL;

    TEST_CALL_ARGS(style_is, div, &after);

    (void) lxb_html_document_stylesheet_remove(document, sst);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(remove_changed_class)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_node_t *div;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *red, *width;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<div class=a>x</div>");
    static const lexbor_str_t red_css = lexbor_str(".a {color: red}");
    static const lexbor_str_t width_css = lexbor_str("div {width: 10px}");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t b_str = lexbor_str("b");
    static const lexbor_str_t before = lexbor_str("color: red; width: 10px");
    static const lexbor_str_t after = lexbor_str("width: 10px");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(attach, document, parser, &red_css);
    red = TEST_RETVAL;

    TEST_CALL_ARGS(attach, document, parser, &width_css);
    width = TEST_RETVAL;

    div = lxb_dom_interface_node(document->body)->first_child;

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         b_str.data, b_str.length);
    test_ne(attr, NULL);

    TEST_CALL_ARGS(style_is, div, &before);

    status = lxb_html_document_stylesheet_remove(document, red);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(style_is, div, &after);

    (void) lxb_css_stylesheet_destroy(red, true);

    TEST_CALL_ARGS(style_is, div, &after);

    status = lxb_html_document_stylesheet_remove(document, width);
    test_eq(status, LXB_STATUS_OK);

    (void) lxb_css_stylesheet_destroy(width, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(style_remove_changed_class)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_node_t *div;
    lxb_css_rule_list_t *list;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<div class=a>x</div>");
    static const lexbor_str_t css = lexbor_str(".a {color: red}"
                                               "div {width: 10px}");

    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t b_str = lexbor_str("b");
    static const lexbor_str_t before = lexbor_str("color: red; width: 10px");
    static const lexbor_str_t after = lexbor_str("width: 10px");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(attach, document, parser, &css);
    sst = TEST_RETVAL;

    div = lxb_dom_interface_node(document->body)->first_child;

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         b_str.data, b_str.length);
    test_ne(attr, NULL);

    TEST_CALL_ARGS(style_is, div, &before);

    list = lxb_css_rule_list(sst->root);

    status = lxb_html_document_style_remove(document,
                                            lxb_css_rule_style(list->first));
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(style_is, div, &after);

    status = lxb_html_document_stylesheet_remove(document, sst);
    test_eq(status, LXB_STATUS_OK);

    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(remove_style_element_changed_class)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_node_t *div, *style;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<style>.a {color: red}</style>"
                                                "<div class=a>x</div>");

    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t b_str = lexbor_str("b");
    static const lexbor_str_t before = lexbor_str("color: red");
    static const lexbor_str_t after = lexbor_str("");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    div = lxb_dom_interface_node(document->body)->first_child;
    style = lxb_dom_interface_node(document->head)->first_child;

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         b_str.data, b_str.length);
    test_ne(attr, NULL);

    TEST_CALL_ARGS(style_is, div, &before);

    lxb_dom_node_remove(style);

    TEST_CALL_ARGS(style_is, div, &after);

    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(removed_element_keeps_inline)
{
    lxb_status_t status;
    lxb_dom_node_t *div;
    lxb_css_parser_t *parser;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<div style='width: 10px'>"
                                                "x</div>");

    static const lexbor_str_t css = lexbor_str("div {color: red}");
    static const lexbor_str_t before = lexbor_str("color: red; width: 10px");
    static const lexbor_str_t after = lexbor_str("width: 10px");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(attach, document, parser, &css);

    div = lxb_dom_interface_node(document->body)->first_child;

    TEST_CALL_ARGS(style_is, div, &before);

    lxb_dom_node_remove(div);

    test_eq(lxb_dom_element_style_by_id(lxb_dom_interface_element(div),
                                        LXB_CSS_PROPERTY_COLOR), NULL);
    test_ne(lxb_dom_element_style_by_id(lxb_dom_interface_element(div),
                                        LXB_CSS_PROPERTY_WIDTH), NULL);

    TEST_CALL_ARGS(style_is, div, &after);

    (void) lxb_html_document_stylesheet_destroy_all(document, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(removed_element_destroy_all)
{
    lxb_status_t status;
    lxb_dom_node_t *div;
    lxb_css_parser_t *parser;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<div>x</div>");
    static const lexbor_str_t css = lexbor_str("div {color: red}");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(attach, document, parser, &css);

    div = lxb_dom_interface_node(document->body)->first_child;

    lxb_dom_node_remove(div);

    lxb_html_document_stylesheet_destroy_all(document, true);

    test_eq(lxb_dom_element_style_by_id(lxb_dom_interface_element(div),
                                        LXB_CSS_PROPERTY_COLOR), NULL);

    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(removed_element_set_style_after_destroy)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_node_t *div;
    lxb_css_parser_t *parser;
    lxb_html_document_t *document;

    static const lexbor_str_t html = lexbor_str("<div>x</div>");
    static const lexbor_str_t css = lexbor_str("div {color: red}");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: blue");
    static const lexbor_str_t after = lexbor_str("color: blue");

    document = lxb_html_document_create();

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(attach, document, parser, &css);

    div = lxb_dom_interface_node(document->body)->first_child;

    lxb_dom_node_remove(div);

    lxb_html_document_stylesheet_destroy_all(document, true);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    TEST_CALL_ARGS(style_is, div, &after);

    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(two_stylesheet_destroy_all);
    TEST_ADD(destroy_all_own_memory);
    TEST_ADD(destroy_all_changed_class);
    TEST_ADD(destroy_all_style_element);
    TEST_ADD(destroy_all_attach);
    TEST_ADD(remove_changed_class);
    TEST_ADD(style_remove_changed_class);
    TEST_ADD(remove_style_element_changed_class);
    TEST_ADD(removed_element_keeps_inline);
    TEST_ADD(removed_element_destroy_all);
    TEST_ADD(removed_element_set_style_after_destroy);

    TEST_RUN("lexbor/style/stylesheet");
    TEST_RELEASE();
}
