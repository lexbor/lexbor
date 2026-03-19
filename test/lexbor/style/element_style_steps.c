/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/style/style.h>
#include <lexbor/selectors/selectors.h>
#include <unit/test.h>


/*
 * Test: Element created with style attribute, not connected to document.
 * Style should remain only on the element (inline only, no stylesheet styles).
 */
TEST_BEGIN(inline_style_not_connected)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *div;
    lxb_html_document_t *document;

    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    /* Only inline style, no stylesheet styles applied. */
    static const lexbor_str_t expected = lexbor_str("color: red");

    /* Create document and init style system. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document,
                                     (lxb_char_t *) "<html><body></body></html>",
                                     25);
    test_eq(status, LXB_STATUS_OK);

    /* Attach stylesheet to document. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    /* Create element with class and style attribute, but do NOT insert. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Element is NOT connected. Style should be inline only. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Cleanup. */

    lxb_dom_document_destroy_interface(lxb_dom_interface_node(div));
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: After connecting element to document, stylesheet styles are applied
 * together with inline styles.
 */
TEST_BEGIN(connect_element_styles_recalc)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    /* Before connection: only inline. */
    static const lexbor_str_t before_str = lexbor_str("color: red");
    /* After connection: inline + stylesheet. */
    static const lexbor_str_t after_str = lexbor_str("color: red; width: 100px");

    /* Create document, init styles, parse HTML. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    /* Attach stylesheet. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    /* Find body element. */

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create element with inline style, not connected. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Check: not connected, inline only. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, before_str.data, before_str.length);

    /* Connect element to body. */

    lxb_html_element_insert_child(body, div);

    /* Check: connected, inline + stylesheet styles applied. */

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, after_str.data, after_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Remove element from document, stylesheet styles removed,
 * inline style attribute remains.
 */
TEST_BEGIN(disconnect_element_keeps_inline)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t connected_str = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t disconnected_str = lexbor_str("color: red");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    /* Find body. */

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create and connect element. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* Connected: inline + stylesheet. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Disconnect element. */

    lxb_html_element_remove(div);

    /* Disconnected: only inline remains. */

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, disconnected_str.data,
                  disconnected_str.length);

    /* Cleanup. */

    lxb_dom_document_destroy_interface(lxb_dom_interface_node(div));
    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Remove style attribute from connected element.
 * Only stylesheet styles should remain.
 */
TEST_BEGIN(remove_style_attr_connected)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t with_inline = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t without_inline = lexbor_str("width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create element, set style, insert into body. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* Connected with inline style. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, with_inline.data, with_inline.length);

    /* Remove style attribute. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(div), attr);
    test_eq(status, LXB_STATUS_OK);

    /* Only stylesheet styles remain. */

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, without_inline.data,
                  without_inline.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Add style attribute back after removing it.
 */
TEST_BEGIN(readd_style_attr)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t with_inline = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t without_inline = lexbor_str("width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create, set style, connect. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* Verify initial state: inline + stylesheet. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, with_inline.data, with_inline.length);

    /* Remove style attr. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(div), attr);
    test_eq(status, LXB_STATUS_OK);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, without_inline.data,
                  without_inline.length);

    /* Add style attr back. */

    status = lxb_dom_element_attr_append(lxb_dom_interface_element(div), attr);
    test_eq(status, LXB_STATUS_OK);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, with_inline.data, with_inline.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Disconnect and reconnect element. Styles recalculated on reconnection.
 */
TEST_BEGIN(reconnect_element)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t connected_str = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t disconnected_str = lexbor_str("color: red");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create, style, insert. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* 1. Connected. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* 2. Disconnect. */

    lxb_dom_node_remove(lxb_dom_interface_node(div));

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, disconnected_str.data,
                  disconnected_str.length);

    /* 3. Reconnect. */

    lxb_html_element_insert_child(body, div);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Change style attribute value on a connected element.
 * Old inline styles replaced, stylesheet styles stay.
 */
TEST_BEGIN(change_style_attr_value)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value1 = lexbor_str("color: red");
    static const lexbor_str_t style_value2 = lexbor_str("height: 50px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t before_change = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t after_change = lexbor_str("height: 50px; width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create, set initial style, connect. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value1.data, style_value1.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* Check before change. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, before_change.data,
                  before_change.length);

    /* Change style attribute value. */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value2.data, style_value2.length);
    test_ne(attr, NULL);

    /* Check after change: old inline gone, new inline + stylesheet. */

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, after_change.data,
                  after_change.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Inline style has higher specificity and overrides stylesheet property.
 */
TEST_BEGIN(inline_overrides_stylesheet)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    /* Inline style sets same property as stylesheet. */
    static const lexbor_str_t style_value = lexbor_str("width: 200px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    /* Inline should win. */
    static const lexbor_str_t expected = lexbor_str("width: 200px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* Inline width: 200px should override stylesheet width: 100px. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Remove inline style that was overriding stylesheet.
 * Stylesheet value should become active.
 */
TEST_BEGIN(remove_inline_reveals_stylesheet)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("width: 200px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t with_override = lexbor_str("width: 200px");
    static const lexbor_str_t after_remove = lexbor_str("width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* Inline overrides: width: 200px. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, with_override.data,
                  with_override.length);

    /* Remove style attribute. Stylesheet should take over. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(div), attr);
    test_eq(status, LXB_STATUS_OK);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, after_remove.data,
                  after_remove.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Element with no style attribute, no matching stylesheet.
 * Serialization should return empty string.
 */
TEST_BEGIN(no_styles_at_all)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t body_str = lexbor_str("body");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    lxb_html_element_insert_child(body, div);

    /* Element has no styles at all. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(out.length, 0);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Multiple style properties in inline style attribute.
 */
TEST_BEGIN(multiple_inline_properties)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("width: 100px; height: 50px; color: blue");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t expected = lexbor_str("color: blue; height: 50px; width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Element with children. Parent has inline style + stylesheet,
 * child has its own inline style. Disconnect parent, check both.
 */
TEST_BEGIN(parent_child_disconnect)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *parent, *child;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t span_str = lexbor_str("span");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t parent_cls = lexbor_str("parent");
    static const lexbor_str_t child_cls = lexbor_str("child");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t parent_style = lexbor_str("color: red");
    static const lexbor_str_t child_style = lexbor_str("font-size: 14px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.parent {width: 100px}"
                                                   "span.child {height: 50px}");

    static const lexbor_str_t parent_connected = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t child_connected = lexbor_str("font-size: 14px; height: 50px");
    static const lexbor_str_t parent_disconnected = lexbor_str("color: red");

    /*
     * When the parent is removed, all its descendants are marked DIRTY_STYLE.
     * Serialization skips non-inline styles for dirty elements, so only
     * the inline style is visible while disconnected.
     */
    static const lexbor_str_t child_disconnected = lexbor_str("font-size: 14px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create parent div. */

    parent = lxb_html_document_create_element(document, div_str.data,
                                              div_str.length, NULL);
    test_ne(parent, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(parent),
                                         class_str.data, class_str.length,
                                         parent_cls.data, parent_cls.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(parent),
                                         style_name.data, style_name.length,
                                         parent_style.data, parent_style.length);
    test_ne(attr, NULL);

    /* Create child span. */

    child = lxb_html_document_create_element(document, span_str.data,
                                             span_str.length, NULL);
    test_ne(child, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child),
                                         class_str.data, class_str.length,
                                         child_cls.data, child_cls.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child),
                                         style_name.data, style_name.length,
                                         child_style.data, child_style.length);
    test_ne(attr, NULL);

    /* Build tree: body > parent > child. */

    lxb_dom_node_insert_child(lxb_dom_interface_node(parent),
                              lxb_dom_interface_node(child));
    lxb_html_element_insert_child(body, parent);

    /* Check connected state. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(parent, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, parent_connected.data,
                  parent_connected.length);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, child_connected.data,
                  child_connected.length);

    /* Remove parent (and child with it). */

    lxb_dom_node_remove(lxb_dom_interface_node(parent));

    /* Check disconnected state. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(parent, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, parent_disconnected.data,
                  parent_disconnected.length);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, child_disconnected.data,
                  child_disconnected.length);

    /* Reconnect parent. */

    lxb_html_element_insert_child(body, parent);

    /* Check reconnected state. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(parent, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, parent_connected.data,
                  parent_connected.length);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, child_connected.data,
                  child_connected.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Set style attribute on disconnected element, then change it,
 * then connect. Only the final style attribute value should apply.
 */
TEST_BEGIN(change_style_before_connect)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value1 = lexbor_str("color: red");
    static const lexbor_str_t style_value2 = lexbor_str("color: blue; margin: 5px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    /* Disconnected, after second set: only second inline style. */
    static const lexbor_str_t disconnected_str = lexbor_str("color: blue; margin: 5px");
    /* Connected: second inline + stylesheet. */
    static const lexbor_str_t connected_str = lexbor_str("color: blue; margin: 5px; width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create element, set first style. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value1.data, style_value1.length);
    test_ne(attr, NULL);

    /* Change style while disconnected. */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value2.data, style_value2.length);
    test_ne(attr, NULL);

    /* Check disconnected: second value only. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, disconnected_str.data,
                  disconnected_str.length);

    /* Connect to document. */

    lxb_html_element_insert_child(body, div);

    /* Check connected: second inline + stylesheet. */

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Remove style attribute from disconnected element.
 * Element should have no styles.
 */
TEST_BEGIN(remove_style_attr_disconnected)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_html_element_t *div;
    lxb_html_document_t *document;

    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");

    static const lexbor_str_t expected = lexbor_str("color: red");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    /* Set style. */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Verify style is set. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Remove style attribute. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(div), attr);
    test_eq(status, LXB_STATUS_OK);

    /* Should have no styles now. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_size(out.length, 0);

    /* Cleanup. */

    lxb_dom_document_destroy_interface(lxb_dom_interface_node(div));
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Element parsed from HTML with inline style and stylesheet.
 * Verify styles are applied correctly during parsing.
 */
TEST_BEGIN(parsed_element_with_inline_and_stylesheet)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<div class=target style='color: red'>text</div>");
    static const lexbor_str_t target_str = lexbor_str("target");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px; height: 50px}");

    static const lexbor_str_t expected = lexbor_str("color: red; height: 50px; width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    /* Find the parsed element. */

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_class_name(lxb_dom_interface_node(document),
                                        collection, target_str.data,
                                        target_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    div = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Verify inline + stylesheet styles. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Move element between parents.
 * Stylesheet may or may not match differently based on position.
 */
TEST_BEGIN(move_element_between_parents)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div1, *div2, *span;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t span_str = lexbor_str("span");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t a_str = lexbor_str("a");
    static const lexbor_str_t b_str = lexbor_str("b");
    static const lexbor_str_t item_str = lexbor_str("item");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str(
        "div.a span.item {width: 100px}"
        "div.b span.item {height: 200px}");

    static const lexbor_str_t in_a = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t in_b = lexbor_str("color: red; height: 200px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create two parent divs. */

    div1 = lxb_html_document_create_element(document, div_str.data,
                                            div_str.length, NULL);
    test_ne(div1, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div1),
                                         class_str.data, class_str.length,
                                         a_str.data, a_str.length);
    test_ne(attr, NULL);

    div2 = lxb_html_document_create_element(document, div_str.data,
                                            div_str.length, NULL);
    test_ne(div2, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div2),
                                         class_str.data, class_str.length,
                                         b_str.data, b_str.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div1);
    lxb_html_element_insert_child(body, div2);

    /* Create span.item with inline style. */

    span = lxb_html_document_create_element(document, span_str.data,
                                            span_str.length, NULL);
    test_ne(span, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(span),
                                         class_str.data, class_str.length,
                                         item_str.data, item_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(span),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Insert into div.a */

    lxb_html_element_insert_child(div1, span);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(span, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, in_a.data, in_a.length);

    /* Move to div.b: remove first, then insert. */

    lxb_html_element_remove(span);
    lxb_html_element_insert_child(div2, span);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(span, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, in_b.data, in_b.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Destroy element that has inline style and is connected.
 * Should not crash.
 */
TEST_BEGIN(destroy_connected_styled_element)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red; width: 50px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {height: 100px}");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* Destroy element while connected. Should not crash. */

    lxb_dom_node_destroy(lxb_dom_interface_node(div));

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Remove style attribute on connected element with stylesheet.
 * Verify that only stylesheet styles remain after removal.
 */
TEST_BEGIN(remove_style_attr_connected_with_stylesheet)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t with_inline = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t without_inline = lexbor_str("width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* Connected with inline. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, with_inline.data, with_inline.length);

    /* Remove style attribute. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(div), attr);
    test_eq(status, LXB_STATUS_OK);

    /* Only stylesheet remains. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, without_inline.data,
                  without_inline.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Full lifecycle. Create -> set style -> connect -> change style ->
 * remove style -> set style again -> disconnect -> reconnect -> destroy.
 */
TEST_BEGIN(full_lifecycle)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_v1 = lexbor_str("color: red");
    static const lexbor_str_t style_v2 = lexbor_str("color: blue; margin: 10px");
    static const lexbor_str_t style_v3 = lexbor_str("padding: 5px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    /* Step 1: disconnected with style_v1. */
    static const lexbor_str_t step1 = lexbor_str("color: red");
    /* Step 2: connected with style_v1 + stylesheet. */
    static const lexbor_str_t step2 = lexbor_str("color: red; width: 100px");
    /* Step 3: connected, style changed to style_v2. */
    static const lexbor_str_t step3 = lexbor_str("color: blue; margin: 10px; width: 100px");
    /* Step 4: connected, style removed. */
    static const lexbor_str_t step4 = lexbor_str("width: 100px");
    /* Step 5: connected, new style_v3 set. */
    static const lexbor_str_t step5 = lexbor_str("padding: 5px; width: 100px");
    /* Step 6: disconnected. */
    static const lexbor_str_t step6 = lexbor_str("padding: 5px");
    /* Step 7: reconnected. */
    static const lexbor_str_t step7 = lexbor_str("padding: 5px; width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Step 1: Create + set style (disconnected). */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_v1.data, style_v1.length);
    test_ne(attr, NULL);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, step1.data, step1.length);

    /* Step 2: Connect. */

    lxb_html_element_insert_child(body, div);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, step2.data, step2.length);

    /* Step 3: Change style value. */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_v2.data, style_v2.length);
    test_ne(attr, NULL);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, step3.data, step3.length);

    /* Step 4: Remove style attribute. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(div), attr);
    test_eq(status, LXB_STATUS_OK);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, step4.data, step4.length);

    /* Step 5: Set new style attribute. */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_v3.data, style_v3.length);
    test_ne(attr, NULL);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, step5.data, step5.length);

    /* Step 6: Disconnect. */

    lxb_dom_node_remove(lxb_dom_interface_node(div));

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, step6.data, step6.length);

    /* Step 7: Reconnect. */

    lxb_html_element_insert_child(body, div);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, step7.data, step7.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Children's lazy cleanup when parent is removed and reinserted
 * into a different context.
 *
 * Scenario:
 * 1. body > div.a > parent > child1 + child2 — children get context-A styles.
 * 2. Remove parent — children are marked DIRTY_STYLE. Serialization skips
 *    non-inline styles for dirty elements, so only inline styles are visible.
 * 3. Reinsert parent into div.b — inserted_steps fires for each descendant,
 *    dirty flag is processed: old stylesheet styles removed, new context-B
 *    styles attached.
 * 4. Verify children have correct (fresh) context-B stylesheet styles.
 */
TEST_BEGIN(dirty_children_lazy_cleanup)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *ctx_a, *ctx_b, *parent, *child1, *child2;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t span_str = lexbor_str("span");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t a_str = lexbor_str("a");
    static const lexbor_str_t b_str = lexbor_str("b");
    static const lexbor_str_t wrap_str = lexbor_str("wrap");
    static const lexbor_str_t c1_str = lexbor_str("c1");
    static const lexbor_str_t c2_str = lexbor_str("c2");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t c1_style = lexbor_str("color: red");
    static const lexbor_str_t c2_style = lexbor_str("color: blue");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str(
        "div.a span.c1 {width: 10px}"
        "div.a span.c2 {width: 20px}"
        "div.b span.c1 {height: 30px}"
        "div.b span.c2 {height: 40px}");

    /* In context div.a. */
    static const lexbor_str_t c1_in_a = lexbor_str("color: red; width: 10px");
    static const lexbor_str_t c2_in_a = lexbor_str("color: blue; width: 20px");

    /*
     * After removing parent from div.a — children are dirty.
     * Serialization filters out non-inline styles for dirty elements,
     * so only inline styles are visible.
     */
    static const lexbor_str_t c1_dirty = lexbor_str("color: red");
    static const lexbor_str_t c2_dirty = lexbor_str("color: blue");

    /* After reinserting parent into div.b — fresh styles from new context. */
    static const lexbor_str_t c1_in_b = lexbor_str("color: red; height: 30px");
    static const lexbor_str_t c2_in_b = lexbor_str("color: blue; height: 40px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create two contexts: div.a and div.b. */

    ctx_a = lxb_html_document_create_element(document, div_str.data,
                                             div_str.length, NULL);
    test_ne(ctx_a, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(ctx_a),
                                         class_str.data, class_str.length,
                                         a_str.data, a_str.length);
    test_ne(attr, NULL);

    ctx_b = lxb_html_document_create_element(document, div_str.data,
                                             div_str.length, NULL);
    test_ne(ctx_b, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(ctx_b),
                                         class_str.data, class_str.length,
                                         b_str.data, b_str.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, ctx_a);
    lxb_html_element_insert_child(body, ctx_b);

    /* Create parent wrapper with two children. */

    parent = lxb_html_document_create_element(document, div_str.data,
                                              div_str.length, NULL);
    test_ne(parent, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(parent),
                                         class_str.data, class_str.length,
                                         wrap_str.data, wrap_str.length);
    test_ne(attr, NULL);

    child1 = lxb_html_document_create_element(document, span_str.data,
                                              span_str.length, NULL);
    test_ne(child1, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child1),
                                         class_str.data, class_str.length,
                                         c1_str.data, c1_str.length);
    test_ne(attr, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child1),
                                         style_name.data, style_name.length,
                                         c1_style.data, c1_style.length);
    test_ne(attr, NULL);

    child2 = lxb_html_document_create_element(document, span_str.data,
                                              span_str.length, NULL);
    test_ne(child2, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child2),
                                         class_str.data, class_str.length,
                                         c2_str.data, c2_str.length);
    test_ne(attr, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child2),
                                         style_name.data, style_name.length,
                                         c2_style.data, c2_style.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(parent, child1);
    lxb_html_element_insert_child(parent, child2);

    /* Step 1: Insert parent into div.a — children get context-A styles. */

    lxb_html_element_insert_child(ctx_a, parent);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child1, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c1_in_a.data, c1_in_a.length);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child2, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c2_in_a.data, c2_in_a.length);

    /*
     * Step 2: Remove parent from div.a.
     * Children are marked DIRTY_STYLE. Serialization checks the dirty flag
     * and skips non-inline (stylesheet) styles — only inline styles visible.
     */

    lxb_html_element_remove(parent);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child1, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c1_dirty.data, c1_dirty.length);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child2, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c2_dirty.data, c2_dirty.length);

    /*
     * Step 3: Reinsert parent into div.b (different context).
     * inserted_steps fires for parent and each child.
     * Dirty flag is processed: old context-A styles removed,
     * new context-B styles attached.
     */

    lxb_html_element_insert_child(ctx_b, parent);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child1, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c1_in_b.data, c1_in_b.length);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child2, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c2_in_b.data, c2_in_b.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Element with only stylesheet styles (no inline), remove and reinsert.
 * The dirty path must clean old stylesheet styles even when there is no inline
 * style to preserve, and re-attach correct stylesheet styles.
 */
TEST_BEGIN(dirty_no_inline_only_stylesheet)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t connected_str = lexbor_str("width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create element with class but NO style attribute. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    /* Connect: stylesheet styles apply. */

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Remove: should have no styles. */

    lxb_html_element_remove(div);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq(out.length, 0);

    /* Reinsert: stylesheet styles must re-apply, not duplicate. */

    lxb_html_element_insert_child(body, div);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Move parent with child to different position. Child has
 * context-dependent selector (e.g. "div.a > span" vs "div.b > span").
 * After move, the child must get styles from the NEW context, not keep
 * old ones.
 */
TEST_BEGIN(dirty_child_context_dependent_move)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div_a, *div_b, *wrapper, *child;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t span_str = lexbor_str("span");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t a_str = lexbor_str("a");
    static const lexbor_str_t b_str = lexbor_str("b");
    static const lexbor_str_t wrap_str = lexbor_str("wrap");
    static const lexbor_str_t item_str = lexbor_str("item");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t child_inline = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str(
        "div.a span.item {width: 100px}"
        "div.b span.item {height: 200px}");

    /* Child in div.a context: inline + width from stylesheet. */
    static const lexbor_str_t in_a = lexbor_str("color: red; width: 100px");
    /* Child in div.b context: inline + height from stylesheet. */
    static const lexbor_str_t in_b = lexbor_str("color: red; height: 200px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create div.a and div.b containers. */

    div_a = lxb_html_document_create_element(document, div_str.data,
                                             div_str.length, NULL);
    test_ne(div_a, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div_a),
                                         class_str.data, class_str.length,
                                         a_str.data, a_str.length);
    test_ne(attr, NULL);

    div_b = lxb_html_document_create_element(document, div_str.data,
                                             div_str.length, NULL);
    test_ne(div_b, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div_b),
                                         class_str.data, class_str.length,
                                         b_str.data, b_str.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div_a);
    lxb_html_element_insert_child(body, div_b);

    /* Create wrapper with child inside. */

    wrapper = lxb_html_document_create_element(document, div_str.data,
                                               div_str.length, NULL);
    test_ne(wrapper, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(wrapper),
                                         class_str.data, class_str.length,
                                         wrap_str.data, wrap_str.length);
    test_ne(attr, NULL);

    child = lxb_html_document_create_element(document, span_str.data,
                                             span_str.length, NULL);
    test_ne(child, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child),
                                         class_str.data, class_str.length,
                                         item_str.data, item_str.length);
    test_ne(attr, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child),
                                         style_name.data, style_name.length,
                                         child_inline.data, child_inline.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(wrapper, child);

    /* Insert wrapper into div.a. */

    lxb_html_element_insert_child(div_a, wrapper);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, in_a.data, in_a.length);

    /* Move wrapper (with child) from div.a to div.b. */

    lxb_html_element_remove(wrapper);
    lxb_html_element_insert_child(div_b, wrapper);

    /*
     * Child's styles must reflect new context: old "width: 100px" from div.a
     * rule must be gone, "height: 200px" from div.b rule must appear.
     */
    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, in_b.data, in_b.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Grandchild (3 levels deep) with context-dependent selector.
 * Remove grandparent, reinsert into different context.
 * Verify that the deeply nested child's styles are correctly recalculated.
 */
TEST_BEGIN(dirty_deeply_nested_grandchild)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div_a, *div_b, *mid, *leaf;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t span_str = lexbor_str("span");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t a_str = lexbor_str("a");
    static const lexbor_str_t b_str = lexbor_str("b");
    static const lexbor_str_t mid_str = lexbor_str("mid");
    static const lexbor_str_t leaf_str = lexbor_str("leaf");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t leaf_style = lexbor_str("color: green");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str(
        "div.a span.leaf {width: 10px}"
        "div.b span.leaf {height: 20px}");

    static const lexbor_str_t in_a = lexbor_str("color: green; width: 10px");
    static const lexbor_str_t in_b = lexbor_str("color: green; height: 20px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create containers. */

    div_a = lxb_html_document_create_element(document, div_str.data,
                                             div_str.length, NULL);
    test_ne(div_a, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div_a),
                                         class_str.data, class_str.length,
                                         a_str.data, a_str.length);
    test_ne(attr, NULL);

    div_b = lxb_html_document_create_element(document, div_str.data,
                                             div_str.length, NULL);
    test_ne(div_b, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div_b),
                                         class_str.data, class_str.length,
                                         b_str.data, b_str.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div_a);
    lxb_html_element_insert_child(body, div_b);

    /* Create: mid > leaf (mid is the subtree root we'll move). */

    mid = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(mid, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(mid),
                                         class_str.data, class_str.length,
                                         mid_str.data, mid_str.length);
    test_ne(attr, NULL);

    leaf = lxb_html_document_create_element(document, span_str.data,
                                            span_str.length, NULL);
    test_ne(leaf, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(leaf),
                                         class_str.data, class_str.length,
                                         leaf_str.data, leaf_str.length);
    test_ne(attr, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(leaf),
                                         style_name.data, style_name.length,
                                         leaf_style.data, leaf_style.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(mid, leaf);

    /* Insert mid into div.a. */

    lxb_html_element_insert_child(div_a, mid);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(leaf, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, in_a.data, in_a.length);

    /* Move mid subtree from div.a to div.b. */

    lxb_html_element_remove(mid);
    lxb_html_element_insert_child(div_b, mid);

    /* Leaf must have new context styles. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(leaf, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, in_b.data, in_b.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Multiple rapid remove/insert cycles. Each cycle should produce
 * clean styles without accumulation of stale entries.
 */
TEST_BEGIN(dirty_multiple_cycles)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t connected_str = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t disconnected_str = lexbor_str("color: red");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Run 5 remove/insert cycles. */

    for (int i = 0; i < 5; i++) {
        lxb_html_element_insert_child(body, div);

        out.data = NULL;
        status = lxb_html_element_style_serialize_str(div, &out,
                                                      LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str_n(out.data, out.length, connected_str.data,
                      connected_str.length);

        lxb_html_element_remove(div);

        out.length = 0;
        status = lxb_html_element_style_serialize_str(div, &out,
                                                      LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str_n(out.data, out.length, disconnected_str.data,
                      disconnected_str.length);
    }

    /* Final insert — styles must be clean. */

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Modify inline style while element is disconnected (dirty),
 * then reinsert. The new inline style should be preserved alongside
 * re-attached stylesheet styles.
 */
TEST_BEGIN(dirty_modify_inline_while_disconnected)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_v1 = lexbor_str("color: red");
    static const lexbor_str_t style_v2 = lexbor_str("margin: 5px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    /* Before disconnect: v1 + stylesheet. */
    static const lexbor_str_t before_str = lexbor_str("color: red; width: 100px");
    /* While disconnected after style change: only v2 (no stylesheet). */
    static const lexbor_str_t disconn_str = lexbor_str("margin: 5px");
    /* After reconnect: v2 + stylesheet. */
    static const lexbor_str_t after_str = lexbor_str("margin: 5px; width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create, style, connect. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_v1.data, style_v1.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, before_str.data, before_str.length);

    /* Disconnect. */

    lxb_html_element_remove(div);

    /* Change style attribute while disconnected (element is dirty). */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_v2.data, style_v2.length);
    test_ne(attr, NULL);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, disconn_str.data, disconn_str.length);

    /* Reconnect: new inline must be preserved, stylesheet re-attached. */

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, after_str.data, after_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Add a new stylesheet while element is disconnected (dirty).
 * On reconnect, the element should get styles from BOTH the old
 * and new stylesheets.
 */
TEST_BEGIN(dirty_new_stylesheet_while_disconnected)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst1, *sst2;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs1 = lexbor_str("div.target {width: 100px}");
    static const lexbor_str_t slctrs2 = lexbor_str("div.target {height: 50px}");

    /* Connected with sst1 only. */
    static const lexbor_str_t with_sst1 = lexbor_str("color: red; width: 100px");
    /* Disconnected: only inline. */
    static const lexbor_str_t disconn_str = lexbor_str("color: red");
    /* Reconnected with both sst1 and sst2. */
    static const lexbor_str_t with_both = lexbor_str("color: red; height: 50px; width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    /* First stylesheet. */

    sst1 = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst1, parser, slctrs1.data,
                                      slctrs1.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst1);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create, style, connect. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, with_sst1.data, with_sst1.length);

    /* Disconnect. */

    lxb_html_element_remove(div);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, disconn_str.data, disconn_str.length);

    /* Add second stylesheet while element is disconnected. */

    sst2 = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst2, parser, slctrs2.data,
                                      slctrs2.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst2);
    test_eq(status, LXB_STATUS_OK);

    /* Reconnect: should get styles from both stylesheets. */

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, with_both.data, with_both.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst1, true);
    (void) lxb_css_stylesheet_destroy(sst2, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Remove child from parent subtree while parent is disconnected,
 * then reinsert parent. The detached child must NOT get styles re-applied
 * (it's no longer in the subtree).
 */
TEST_BEGIN(dirty_child_detached_before_parent_reinsert)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *parent, *child;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t span_str = lexbor_str("span");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t parent_cls = lexbor_str("parent");
    static const lexbor_str_t child_cls = lexbor_str("child");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t child_style = lexbor_str("color: blue");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str(
        "div.parent {width: 100px}"
        "div.parent span.child {height: 50px}");

    static const lexbor_str_t parent_connected = lexbor_str("width: 100px");
    static const lexbor_str_t child_connected = lexbor_str("color: blue; height: 50px");
    /* Child after detaching from parent (dirty, but no parent context). */
    static const lexbor_str_t child_detached = lexbor_str("color: blue");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create parent > child, connect to body. */

    parent = lxb_html_document_create_element(document, div_str.data,
                                              div_str.length, NULL);
    test_ne(parent, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(parent),
                                         class_str.data, class_str.length,
                                         parent_cls.data, parent_cls.length);
    test_ne(attr, NULL);

    child = lxb_html_document_create_element(document, span_str.data,
                                             span_str.length, NULL);
    test_ne(child, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child),
                                         class_str.data, class_str.length,
                                         child_cls.data, child_cls.length);
    test_ne(attr, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child),
                                         style_name.data, style_name.length,
                                         child_style.data, child_style.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(parent, child);
    lxb_html_element_insert_child(body, parent);

    /* Verify connected state. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, child_connected.data,
                  child_connected.length);

    /* Disconnect parent (child becomes dirty). */

    lxb_html_element_remove(parent);

    /* Detach child from parent while parent is disconnected. */

    lxb_dom_node_remove(lxb_dom_interface_node(child));

    /* Reinsert parent (without child). */

    lxb_html_element_insert_child(body, parent);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(parent, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, parent_connected.data,
                  parent_connected.length);

    /*
     * The child is still detached. It was marked dirty when parent was
     * removed. Now reinsert child directly into body (no parent context).
     * "div.parent span.child" should NOT match.
     */

    lxb_html_element_insert_child(body, child);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, child_detached.data,
                  child_detached.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Remove inline style attribute while element is disconnected (dirty),
 * then reinsert. Only stylesheet styles should remain.
 */
TEST_BEGIN(dirty_remove_inline_while_disconnected)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t connected_str = lexbor_str("color: red; width: 100px");
    /* After reconnect with inline removed: only stylesheet. */
    static const lexbor_str_t after_str = lexbor_str("width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create, style, connect. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Disconnect. */

    lxb_html_element_remove(div);

    /* Remove style attribute while disconnected. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(div), attr);
    test_eq(status, LXB_STATUS_OK);

    /* Verify: no styles at all when disconnected with no inline. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq(out.length, 0);

    /* Reconnect: only stylesheet styles should apply. */

    lxb_html_element_insert_child(body, div);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, after_str.data, after_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Selector that no longer matches after move.
 * Element has class-based selector, remove class while disconnected,
 * reinsert — stylesheet styles should NOT apply.
 */
TEST_BEGIN(dirty_class_removed_while_disconnected)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr, *cls_attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    static const lexbor_str_t connected_str = lexbor_str("color: red; width: 100px");
    /* After removing class and reconnecting: no selector match, only inline. */
    static const lexbor_str_t after_str = lexbor_str("color: red");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    cls_attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                              class_str.data, class_str.length,
                                              target_str.data, target_str.length);
    test_ne(cls_attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Disconnect. */

    lxb_html_element_remove(div);

    /* Remove class attribute while disconnected. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(div),
                                          cls_attr);
    test_eq(status, LXB_STATUS_OK);

    /* Reconnect: selector "div.target" should no longer match. */

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, after_str.data, after_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Two siblings under same parent, both matching same selector.
 * Remove parent, reinsert. Both siblings must independently get correct
 * styles back.
 */
TEST_BEGIN(dirty_multiple_children_reinsert)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *parent, *child1, *child2;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t span_str = lexbor_str("span");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t parent_cls = lexbor_str("parent");
    static const lexbor_str_t item_str = lexbor_str("item");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_v1 = lexbor_str("color: red");
    static const lexbor_str_t style_v2 = lexbor_str("color: blue");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str(
        "div.parent span.item {width: 100px}");

    static const lexbor_str_t c1_connected = lexbor_str("color: red; width: 100px");
    static const lexbor_str_t c2_connected = lexbor_str("color: blue; width: 100px");
    static const lexbor_str_t c1_disconnected = lexbor_str("color: red");
    static const lexbor_str_t c2_disconnected = lexbor_str("color: blue");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create parent. */

    parent = lxb_html_document_create_element(document, div_str.data,
                                              div_str.length, NULL);
    test_ne(parent, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(parent),
                                         class_str.data, class_str.length,
                                         parent_cls.data, parent_cls.length);
    test_ne(attr, NULL);

    /* Create two children. */

    child1 = lxb_html_document_create_element(document, span_str.data,
                                              span_str.length, NULL);
    test_ne(child1, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child1),
                                         class_str.data, class_str.length,
                                         item_str.data, item_str.length);
    test_ne(attr, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child1),
                                         style_name.data, style_name.length,
                                         style_v1.data, style_v1.length);
    test_ne(attr, NULL);

    child2 = lxb_html_document_create_element(document, span_str.data,
                                              span_str.length, NULL);
    test_ne(child2, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child2),
                                         class_str.data, class_str.length,
                                         item_str.data, item_str.length);
    test_ne(attr, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child2),
                                         style_name.data, style_name.length,
                                         style_v2.data, style_v2.length);
    test_ne(attr, NULL);

    /* Build tree: parent > child1 + child2. */

    lxb_html_element_insert_child(parent, child1);
    lxb_html_element_insert_child(parent, child2);
    lxb_html_element_insert_child(body, parent);

    /* Verify connected state for both children. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child1, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c1_connected.data,
                  c1_connected.length);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child2, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c2_connected.data,
                  c2_connected.length);

    /* Remove parent. */

    lxb_html_element_remove(parent);

    /* Verify disconnected state: stylesheet styles should be gone. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child1, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c1_disconnected.data,
                  c1_disconnected.length);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child2, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c2_disconnected.data,
                  c2_disconnected.length);

    /* Reinsert parent. */

    lxb_html_element_insert_child(body, parent);

    /* Both children must have styles correctly re-applied. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child1, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c1_connected.data,
                  c1_connected.length);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child2, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, c2_connected.data,
                  c2_connected.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Inline style overrides same property from stylesheet.
 * Remove element, reinsert. After reinsertion, inline must still override.
 */
TEST_BEGIN(dirty_inline_override_preserved_after_reinsert)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    /* Inline sets the same property as stylesheet but different value. */
    static const lexbor_str_t style_value = lexbor_str("width: 200px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    /* Inline should win over stylesheet. */
    static const lexbor_str_t expected = lexbor_str("width: 200px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div);

    /* Connected: inline width:200px should override stylesheet width:100px. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Remove and reinsert. */

    lxb_html_element_remove(div);
    lxb_html_element_insert_child(body, div);

    /* After dirty path: inline must still override. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Do it again to verify consistency. */

    lxb_html_element_remove(div);
    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END


/*
 * Test: Stylesheet with !important should override inline style for the same
 * property. After remove/reinsert cycle, the !important stylesheet value
 * must still win over inline.
 */
TEST_BEGIN(important_stylesheet_overrides_inline)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("width: 200px");
    static const lexbor_str_t body_str = lexbor_str("body");

    /* Stylesheet with !important on the same property. */
    static const lexbor_str_t slctrs =
        lexbor_str("div.target {width: 100px !important}");

    /*
     * !important from stylesheet (sp_i=1, bits 28+) beats
     * inline style (sp_s=1, bit 27). So width should be 100px !important.
     */
    static const lexbor_str_t expected =
        lexbor_str("width: 100px !important");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Connect: !important from stylesheet must override inline width:200px. */

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Remove and reinsert: !important must still win. */

    lxb_html_element_remove(div);
    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Removing a stylesheet from the document should remove its styles
 * from all matching elements. Element with both inline and stylesheet styles
 * should keep only inline after stylesheet removal.
 */
TEST_BEGIN(stylesheet_removal_cleans_elements)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs =
        lexbor_str("div.target {width: 100px; height: 50px}");

    /* Connected with both inline and stylesheet. */
    static const lexbor_str_t both =
        lexbor_str("color: red; height: 50px; width: 100px");

    /* After stylesheet removal, only inline style remains. */
    static const lexbor_str_t only_inline = lexbor_str("color: red");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Connect: both inline and stylesheet styles. */

    lxb_html_element_insert_child(body, div);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, both.data, both.length);

    /* Remove the stylesheet from document. */

    status = lxb_html_document_stylesheet_remove(document, sst);
    test_eq(status, LXB_STATUS_OK);

    /* Element should now have only inline style. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, only_inline.data, only_inline.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Element with NO inline style, only stylesheet, moved between
 * different selector contexts (div.a > span.item vs div.b > span.item).
 * Unlike dirty_child_context_dependent_move which has inline style,
 * this tests that dirty cleanup correctly handles the case where ALL
 * styles are from stylesheets and must be fully replaced.
 */
TEST_BEGIN(dirty_no_inline_context_move)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *div_a, *div_b, *child;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t span_str = lexbor_str("span");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t a_str = lexbor_str("a");
    static const lexbor_str_t b_str = lexbor_str("b");
    static const lexbor_str_t item_str = lexbor_str("item");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str(
        "div.a span.item {width: 100px}"
        "div.b span.item {height: 200px}");

    /* Child in div.a: only stylesheet width. */
    static const lexbor_str_t in_a = lexbor_str("width: 100px");
    /* Child in div.b: only stylesheet height. */
    static const lexbor_str_t in_b = lexbor_str("height: 200px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create div.a and div.b containers. */

    div_a = lxb_html_document_create_element(document, div_str.data,
                                             div_str.length, NULL);
    test_ne(div_a, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div_a),
                                         class_str.data, class_str.length,
                                         a_str.data, a_str.length);
    test_ne(attr, NULL);

    div_b = lxb_html_document_create_element(document, div_str.data,
                                             div_str.length, NULL);
    test_ne(div_b, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div_b),
                                         class_str.data, class_str.length,
                                         b_str.data, b_str.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, div_a);
    lxb_html_element_insert_child(body, div_b);

    /* Create child span.item with NO inline style. */

    child = lxb_html_document_create_element(document, span_str.data,
                                             span_str.length, NULL);
    test_ne(child, NULL);
    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child),
                                         class_str.data, class_str.length,
                                         item_str.data, item_str.length);
    test_ne(attr, NULL);

    /* Insert into div.a: gets width:100px from stylesheet. */

    lxb_html_element_insert_child(div_a, child);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, in_a.data, in_a.length);

    /* Move to div.b: old width must be gone, height:200px must appear. */

    lxb_html_element_remove(child);

    /* While disconnected: no styles (dirty, all were from stylesheet). */

    out.length = 0;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq(out.length, 0);

    /* Insert into div.b. */

    lxb_html_element_insert_child(div_b, child);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, in_b.data, in_b.length);

    /* Move back to div.a to verify consistency. */

    lxb_html_element_remove(child);
    lxb_html_element_insert_child(div_a, child);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, in_a.data, in_a.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END


/*
 * Test: Unknown element (tag not in HTML spec) with inline style, not connected.
 * Style attribute must be parsed and applied just like for known elements.
 */
TEST_BEGIN(unknown_element_inline_not_connected)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *el;
    lxb_html_document_t *document;

    static const lexbor_str_t tag_str = lexbor_str("mytag");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");

    static const lexbor_str_t slctrs = lexbor_str("mytag {width: 100px}");

    /* Not connected: only inline style, no stylesheet. */
    static const lexbor_str_t expected = lexbor_str("color: red");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document,
                                     (lxb_char_t *) "<html><body></body></html>",
                                     25);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    /* Create unknown element with style attribute, do NOT insert. */

    el = lxb_html_document_create_element(document, tag_str.data,
                                          tag_str.length, NULL);
    test_ne(el, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(el),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Not connected: only inline. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, expected.data, expected.length);

    /* Cleanup. */

    lxb_dom_document_destroy_interface(lxb_dom_interface_node(el));
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Unknown element connected to document receives stylesheet styles.
 * Verifies that inserted_steps fires for unknown tags and attaches
 * matching stylesheet rules.
 */
TEST_BEGIN(unknown_element_stylesheet_connected)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *el;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t tag_str = lexbor_str("mycustom");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs =
        lexbor_str("mycustom {width: 100px; height: 50px}");

    /* Connected: inline + stylesheet properties. */
    static const lexbor_str_t connected =
        lexbor_str("color: red; height: 50px; width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    el = lxb_html_document_create_element(document, tag_str.data,
                                          tag_str.length, NULL);
    test_ne(el, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(el),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Connect: both inline and stylesheet styles must apply. */

    lxb_html_element_insert_child(body, el);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected.data, connected.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Unknown element remove/reinsert triggers dirty style path.
 * After removal, only inline styles should be visible (stylesheet filtered
 * by dirty flag). After reinsertion, stylesheet styles must re-apply.
 */
TEST_BEGIN(unknown_element_dirty_reinsert)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *el;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t tag_str = lexbor_str("xtag");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("xtag {width: 100px}");

    static const lexbor_str_t connected_str =
        lexbor_str("color: red; width: 100px");
    /* Disconnected (dirty): only inline style visible. */
    static const lexbor_str_t disconnected_str = lexbor_str("color: red");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    el = lxb_html_document_create_element(document, tag_str.data,
                                          tag_str.length, NULL);
    test_ne(el, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(el),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* Connect. */

    lxb_html_element_insert_child(body, el);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Remove: dirty flag set, only inline visible. */

    lxb_html_element_remove(el);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, disconnected_str.data,
                  disconnected_str.length);

    /* Reinsert: dirty cleaned, stylesheet re-attached. */

    lxb_html_element_insert_child(body, el);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Destroy connected unknown element with inline + stylesheet styles.
 * Verifies destroy_steps fires for unknown tags. Must not crash.
 */
TEST_BEGIN(unknown_element_destroy_connected)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *el;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t tag_str = lexbor_str("xtag");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("xtag {width: 100px}");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    el = lxb_html_document_create_element(document, tag_str.data,
                                          tag_str.length, NULL);
    test_ne(el, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(el),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, el);

    /* Destroy while connected. Must not crash. */

    lxb_dom_node_destroy(lxb_dom_interface_node(el));

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Change style attribute on connected unknown element.
 * Verifies attr_change steps fire for unknown tags: old inline styles
 * are removed, new ones parsed and applied, alongside stylesheet styles.
 */
TEST_BEGIN(unknown_element_attr_change)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *el;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t tag_str = lexbor_str("xtag");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_v1 = lexbor_str("color: red");
    static const lexbor_str_t style_v2 = lexbor_str("font-size: 14px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("xtag {width: 100px}");

    /* After first style. */
    static const lexbor_str_t with_v1 =
        lexbor_str("color: red; width: 100px");
    /* After changing to second style. */
    static const lexbor_str_t with_v2 =
        lexbor_str("font-size: 14px; width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    el = lxb_html_document_create_element(document, tag_str.data,
                                          tag_str.length, NULL);
    test_ne(el, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(el),
                                         style_name.data, style_name.length,
                                         style_v1.data, style_v1.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, el);

    /* Connected with first inline style. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, with_v1.data, with_v1.length);

    /* Change style attribute to a different value. */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(el),
                                         style_name.data, style_name.length,
                                         style_v2.data, style_v2.length);
    test_ne(attr, NULL);

    /* Old inline gone, new inline + stylesheet. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, with_v2.data, with_v2.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Remove style attribute on connected unknown element.
 * Verifies attr_remove steps fire: inline style removed, only stylesheet
 * styles remain.
 */
TEST_BEGIN(unknown_element_attr_remove)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *el;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t tag_str = lexbor_str("xtag");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("xtag {width: 100px}");

    /* Connected with both. */
    static const lexbor_str_t both_str =
        lexbor_str("color: red; width: 100px");
    /* After removing style attr: only stylesheet. */
    static const lexbor_str_t only_sheet = lexbor_str("width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    el = lxb_html_document_create_element(document, tag_str.data,
                                          tag_str.length, NULL);
    test_ne(el, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(el),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(body, el);

    /* Connected: both inline and stylesheet. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, both_str.data, both_str.length);

    /* Remove style attribute. */

    lxb_dom_element_remove_attribute(lxb_dom_interface_element(el),
                                     style_name.data, style_name.length);

    /* Only stylesheet styles remain. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, only_sheet.data, only_sheet.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Unknown element as child of known element. Parent removal marks
 * children dirty. Unknown child must have dirty flag set and go through
 * lazy cleanup on re-insertion.
 */
TEST_BEGIN(unknown_element_child_dirty_propagation)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *parent, *child;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t tag_str = lexbor_str("xtag");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t child_inline = lexbor_str("font-size: 14px");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs =
        lexbor_str("div xtag {height: 50px}");

    /* Child connected: inline + stylesheet. */
    static const lexbor_str_t child_connected =
        lexbor_str("font-size: 14px; height: 50px");
    /* Child disconnected (dirty): only inline. */
    static const lexbor_str_t child_disconnected =
        lexbor_str("font-size: 14px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create known parent div. */

    parent = lxb_html_document_create_element(document, div_str.data,
                                              div_str.length, NULL);
    test_ne(parent, NULL);

    /* Create unknown child with inline style. */

    child = lxb_html_document_create_element(document, tag_str.data,
                                             tag_str.length, NULL);
    test_ne(child, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(child),
                                         style_name.data, style_name.length,
                                         child_inline.data, child_inline.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(parent, child);
    lxb_html_element_insert_child(body, parent);

    /* Connected: unknown child has inline + stylesheet. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, child_connected.data,
                  child_connected.length);

    /* Remove parent: unknown child must be marked dirty. */

    lxb_html_element_remove(parent);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, child_disconnected.data,
                  child_disconnected.length);

    /* Reinsert parent: unknown child styles restored. */

    lxb_html_element_insert_child(body, parent);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(child, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, child_connected.data,
                  child_connected.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

/*
 * Test: Unknown element with no inline style, only stylesheet.
 * Remove and reinsert. After removal serialization should be empty
 * (all styles from stylesheet, dirty). After reinsertion, stylesheet
 * styles must re-apply.
 */
TEST_BEGIN(unknown_element_no_inline_dirty)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *body, *el;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    static const lexbor_str_t html = lexbor_str("<html><body></body></html>");
    static const lexbor_str_t tag_str = lexbor_str("xtag");
    static const lexbor_str_t body_str = lexbor_str("body");

    static const lexbor_str_t slctrs = lexbor_str("xtag {width: 100px}");
    static const lexbor_str_t connected_str = lexbor_str("width: 100px");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, body_str.data,
                                      body_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    body = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create unknown element with NO style attribute. */

    el = lxb_html_document_create_element(document, tag_str.data,
                                          tag_str.length, NULL);
    test_ne(el, NULL);

    /* Connect: stylesheet styles apply. */

    lxb_html_element_insert_child(body, el);

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Remove: no styles (all from stylesheet, dirty). */

    lxb_html_element_remove(el);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq(out.length, 0);

    /* Reinsert: stylesheet re-applies. */

    lxb_html_element_insert_child(body, el);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(el, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, connected_str.data,
                  connected_str.length);

    /* Cleanup. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END


int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(inline_style_not_connected);
    TEST_ADD(connect_element_styles_recalc);
    TEST_ADD(disconnect_element_keeps_inline);
    TEST_ADD(remove_style_attr_connected);
    TEST_ADD(readd_style_attr);
    TEST_ADD(reconnect_element);
    TEST_ADD(change_style_attr_value);
    TEST_ADD(inline_overrides_stylesheet);
    TEST_ADD(remove_inline_reveals_stylesheet);
    TEST_ADD(no_styles_at_all);
    TEST_ADD(multiple_inline_properties);
    TEST_ADD(parent_child_disconnect);
    TEST_ADD(change_style_before_connect);
    TEST_ADD(remove_style_attr_disconnected);
    TEST_ADD(parsed_element_with_inline_and_stylesheet);
    TEST_ADD(move_element_between_parents);
    TEST_ADD(destroy_connected_styled_element);
    TEST_ADD(remove_style_attr_connected_with_stylesheet);
    TEST_ADD(full_lifecycle);

    TEST_ADD(dirty_children_lazy_cleanup);
    TEST_ADD(dirty_no_inline_only_stylesheet);
    TEST_ADD(dirty_child_context_dependent_move);
    TEST_ADD(dirty_deeply_nested_grandchild);
    TEST_ADD(dirty_multiple_cycles);
    TEST_ADD(dirty_modify_inline_while_disconnected);
    TEST_ADD(dirty_new_stylesheet_while_disconnected);
    TEST_ADD(dirty_child_detached_before_parent_reinsert);
    TEST_ADD(dirty_remove_inline_while_disconnected);
    TEST_ADD(dirty_class_removed_while_disconnected);
    TEST_ADD(dirty_multiple_children_reinsert);
    TEST_ADD(dirty_inline_override_preserved_after_reinsert);
    TEST_ADD(important_stylesheet_overrides_inline);
    TEST_ADD(stylesheet_removal_cleans_elements);
    TEST_ADD(dirty_no_inline_context_move);

    /* Unknown element tests. */
    TEST_ADD(unknown_element_inline_not_connected);
    TEST_ADD(unknown_element_stylesheet_connected);
    TEST_ADD(unknown_element_dirty_reinsert);
    TEST_ADD(unknown_element_destroy_connected);
    TEST_ADD(unknown_element_attr_change);
    TEST_ADD(unknown_element_attr_remove);
    TEST_ADD(unknown_element_child_dirty_propagation);
    TEST_ADD(unknown_element_no_inline_dirty);

    TEST_RUN("lexbor/style/element_style_steps");
    TEST_RELEASE();
}
