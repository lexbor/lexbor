/*
 * Copyright (C) 2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>

#include <unit/test.h>


TEST_BEGIN(by_class)
{
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    lxb_char_t data[] = "<html><head></head><body>"
                        "<div class='with-accordion'>1</div>"
                        "<div class='with-accordion hidden'>2</div>"
                        "<div class='hidden with-accordion'>3</div>"
                        "<div class='hidden with-accordion hidden'>4</div>"
                        "<div class='    with-accordion'>5</div>"
                        "<div class='with-accordion      '>6</div>"
                        "<div class='hidden    hidden  with-accordion'>7</div>"
                        "<div class='with-accordion    hidden'>8</div>"
                        "<div class='wewith-accordion'></div>"
                        "<div class='with-accordionwe'></div>"
                        "<div class='wewith-accordionwe'></div>"
                        "<div class='with- accordion'></div>"
                        "</body></html>";

    size_t len = sizeof(data) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, data, len);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_class_name(lxb_dom_interface_element(document),
                                            collection,
                                            (lxb_char_t *) "with-accordion", 14);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 8);

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(by_class);

    TEST_RUN("lexbor/html/element_by");
    TEST_RELEASE();
}
