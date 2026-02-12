/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/style/style.h>
#include <unit/test.h>

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

    status = lxb_html_document_css_init(document, true);
    test_eq(status, LXB_STATUS_OK);

    /* Parse HTML. */

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    /* Destroy resources. */
    (void) lxb_html_document_stylesheet_destroy_all(document, true);
    (void) lxb_html_document_css_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(two_stylesheet_destroy_all);

    TEST_RUN("lexbor/style/stylesheet");
    TEST_RELEASE();
}
