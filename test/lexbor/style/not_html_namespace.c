/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/style/style.h>
#include <unit/test.h>


TEST_BEGIN(styles)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_html_element_t *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* HTML Data. */

    static const lexbor_str_t html = lexbor_str("<style>.father {width: 120px}</style>"
                                                "<div class=father><svg style='width: 10em'>"
                                                "<style>.father {height: 380px}</style>"
                                                "<path style='width: 50%' class=father></svg></div>");

    /* Other Data. */

    static const lexbor_str_t father_str = lexbor_str("father");
    static const lexbor_str_t result_str = lexbor_str("width: 120px");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    /* Init all CSS objects and momory for Document. */

    status = lxb_html_document_css_init(document, true);
    test_eq(status, LXB_STATUS_OK);

    /* Parse HTML. */

    status = lxb_html_document_parse(document, html.data, html.length);
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

    /* Tests 1. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(out.data, out.length, result_str.data, result_str.length);

    /* Destroy resources. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_html_document_css_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(styles);

    TEST_RUN("lexbor/style/not_html_namespace");
    TEST_RELEASE();
}
