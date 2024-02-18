/*
 * Copyright (C) 2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>

#include <unit/test.h>

static const lxb_char_t data[] = "<html><head></head><body>"
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
                                 "<div class></div>"
                                 "<div class=''></div>"
                                 "<div id></div>"
                                 "<div id=''></div>"
                                 "<div id='abc'></div>"
                                 "<div foo></div>"
                                 "<div foo=''></div>"
                                 "<div foo='abc'></div>"
                                 "</body></html>";

static const size_t data_len = sizeof(data) - 1;

TEST_BEGIN(by_class)
{
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, data, data_len);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_class_name(lxb_dom_interface_element(document),
                                            collection,
                                            (lxb_char_t *) "with-accordion", 14);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_dom_collection_length(collection), 8);

    // Empty class (doesn't select anything, but shouldn't crash either)
    status = lxb_dom_elements_by_class_name(lxb_dom_interface_element(document),
                                            collection,
                                            (lxb_char_t *) "", 0);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_dom_collection_length(collection), 8);

    // Empty class with NULL
    status = lxb_dom_elements_by_class_name(lxb_dom_interface_element(document),
                                            collection,
                                            (lxb_char_t *) NULL, 0);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_dom_collection_length(collection), 8);

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(by_attr)
{
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, data, data_len);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    // ID
    status = lxb_dom_elements_by_attr(lxb_dom_interface_element(document),
                                      collection,
                                      (lxb_char_t *) "id", 2,
                                      (lxb_char_t *) "abc", 3, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_dom_collection_length(collection), 1);

    // Empty ID
    status = lxb_dom_elements_by_attr(lxb_dom_interface_element(document),
                                      collection,
                                      (lxb_char_t *) "id", 2,
                                      (lxb_char_t *) "", 0, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_dom_collection_length(collection), 3);

    // Arbitrary attribute name
    status = lxb_dom_elements_by_attr(lxb_dom_interface_element(document),
                                      collection,
                                      (lxb_char_t *) "foo", 3,
                                      (lxb_char_t *) "abc", 3, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_dom_collection_length(collection), 4);

    // Empty attribute
    status = lxb_dom_elements_by_attr(lxb_dom_interface_element(document),
                                      collection,
                                      (lxb_char_t *) "foo", 3,
                                      (lxb_char_t *) "", 0, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_dom_collection_length(collection), 6);

    // Empty attribute with NULL
    status = lxb_dom_elements_by_attr(lxb_dom_interface_element(document),
                                      collection,
                                      (lxb_char_t *) "foo", 3,
                                      (lxb_char_t *) NULL, 0, false);
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
    TEST_ADD(by_attr);

    TEST_RUN("lexbor/html/element_by");
    TEST_RELEASE();
}
