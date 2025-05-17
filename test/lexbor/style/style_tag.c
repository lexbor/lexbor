/*
 * Copyright (C) 2023-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/style/style.h>
#include <unit/test.h>


TEST_BEGIN(styles)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *div, *style;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* HTML Data. */

    static const lexbor_str_t html = lexbor_str("<div class=father></div>"
    "<style>div.father {width: 20px; height: 10pt; display: block}</style>");

    /* StyleSheet Data. */

    static const lexbor_str_t slctrs = lexbor_str("div.father {width: 30%}");

    /* Other Data. */

    static const lexbor_str_t father_str = lexbor_str("father");
    static const lexbor_str_t style_str = lexbor_str("style");

    static const lexbor_str_t res_str = lexbor_str("display: block; height: 10pt; width: 30%");
    static const lexbor_str_t style_rm_str = lexbor_str("width: 30%");
    static const lexbor_str_t style_in_str = lexbor_str("display: block; height: 10pt; width: 20px");

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

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(document),
                                      collection, style_str.data,
                                      style_str.length);
    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    div = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));
    style = lxb_html_interface_element(lxb_dom_collection_node(collection, 1));

    /* Tests 1. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(out.data, out.length, res_str.data, res_str.length);

    /* Tests 2. Remove the style element. */

    lxb_html_element_remove(style);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(out.data, out.length, style_rm_str.data, style_rm_str.length);

    /* Tests 3. Insert the style element. */

    lxb_html_element_insert_after(div, style);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(out.data, out.length, style_in_str.data, style_in_str.length);

    /* Tests 4. Remove the style element. */

    lxb_html_element_remove(style);

    out.length = 0;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(out.data, out.length, style_rm_str.data, style_rm_str.length);

    /* Destroy resources. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_html_document_css_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

TEST_BEGIN(bad_styles)
{
    lxb_status_t status;
    lxb_html_document_t *document;

    /* HTML Data. */

    static const lexbor_str_t html = lexbor_str("<div class=father></div>"
                                                "<style>div.father</style>");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    /* Init all CSS objects and momory for Document. */

    status = lxb_html_document_css_init(document, true);
    test_eq(status, LXB_STATUS_OK);

    /* Parse HTML. */

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    /* Destroy resources. */

    (void) lxb_html_document_css_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(styles);
    TEST_ADD(bad_styles);

    TEST_RUN("lexbor/style/style_tag");
    TEST_RELEASE();
}
