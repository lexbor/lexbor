/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/style/style.h>
#include <lexbor/selectors/selectors.h>
#include <unit/test.h>


TEST_BEGIN(styles)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *np, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* HTML Data. */

    static const lexbor_str_t html = lexbor_str("<div class=father>"
                                                "<p class=best'>a</p>"
                                                "<p>b</p>"
                                                "<s>c</s>"
                                                "</div>");
    /* StyleSheet Data. */

    static const lexbor_str_t slctrs = lexbor_str("div.father {width: 30%}"
                                                  "div.father p.best {width: 20px; height: 10pt}");

    /* Other Data. */

    static const lexbor_str_t father_str = lexbor_str("father");
    static const lexbor_str_t p_str = lexbor_str("p");

    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t best_str = lexbor_str("best");

    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("height: 100px");

    static const lexbor_str_t res_str = lexbor_str("height: 100px; width: 20px");
    static const lexbor_str_t attr_rm_str = lexbor_str("height: 10pt; width: 20px");
    static const lexbor_str_t el_rm_str = lexbor_str("height: 100px");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    /* Init all CSS objects and momory for Document. */

    status = lxb_html_document_css_init(document, true);
    test_eq(status, LXB_STATUS_OK);

    /* Parse HTML. */

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    /* Create CSS parser. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_parse(parser, slctrs.data, slctrs.length);
    test_ne(sst, NULL);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    /* Find HTML nodes by CSS Selectors. */

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    test_ne(collection, NULL);

    status = lxb_dom_node_by_class_name(lxb_dom_interface_node(document),
                                        collection, father_str.data,
                                        father_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    div = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Create new element and append in to div as child. */

    np = lxb_html_document_create_element(document, p_str.data, p_str.length,
                                          NULL);
    test_ne(np, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(np),
                                         class_str.data, class_str.length,
                                         best_str.data, best_str.length);
    test_ne(attr, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(np),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    lxb_html_element_insert_child(div, np);

    /* Tests. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(np, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(out.data, out.length, res_str.data, res_str.length);

    /* Remove. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(np), attr);
    test_eq(status, LXB_STATUS_OK);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(np, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(out.data, out.length, attr_rm_str.data, attr_rm_str.length);

    /* Insert back. */

    status = lxb_dom_element_attr_append(lxb_dom_interface_element(np), attr);
    test_eq(status, LXB_STATUS_OK);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(np, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(out.data, out.length, res_str.data, res_str.length);

    /* Remove element. */

    lxb_dom_node_remove(lxb_dom_interface_node(np));

    out.length = 0;
    status = lxb_html_element_style_serialize_str(np, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(out.data, out.length, el_rm_str.data, el_rm_str.length);

    /* Insert back. */

    lxb_html_element_insert_child(div, np);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(np, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(out.data, out.length, res_str.data, res_str.length);

    /* Destroy resources. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_html_document_css_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

TEST_BEGIN(destroy_element_with_inline_style)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_html_element_t *element;
    lxb_html_document_t *document;

    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t style_str = lexbor_str("style");
    static const lexbor_str_t inline_str = lexbor_str("display: flex;");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_css_init(document, true);
    test_eq(status, LXB_STATUS_OK);

    element = lxb_html_document_create_element(document, div_str.data,
                                               div_str.length, NULL);
    test_ne(element, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(element),
                                         style_str.data, style_str.length,
                                         inline_str.data, inline_str.length);
    test_ne(attr, NULL);

    lxb_dom_document_destroy_interface(lxb_dom_interface_node(element));

    (void) lxb_html_document_css_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(styles);
    TEST_ADD(destroy_element_with_inline_style);

    TEST_RUN("lexbor/style/element_events");
    TEST_RELEASE();
}
