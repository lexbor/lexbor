/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>

#include <unit/test.h>


const lxb_char_t html[] = "<div id=my-best-id></div>";

const size_t html_length = sizeof(html) - 1;


TEST_BEGIN(attrs)
{
    size_t i, size;
    lxb_status_t status;
    lxb_html_body_element_t *body;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;
    lxb_dom_element_t *element;
    lxb_dom_attr_t *attr;

    bool is_exist;
    const lxb_char_t *name;
    const lxb_char_t *value;
    size_t value_len;

    static const char *names[] = {"id", "class", "some", NULL};

    /* Parse. */
    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Collection for elements. */
    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    /* Get BODY element (root for search) */
    body = lxb_html_document_body_element(document);
    element = lxb_dom_interface_element(body);

    /* Find DIV eleemnt */
    status = lxb_dom_elements_by_tag_name(element, collection,
                                          (const lxb_char_t *) "div", 3);

    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    /* Append new attribute */
    element = lxb_dom_collection_element(collection, 0);

    i = 0;

    while (names[i] != NULL) {
        name = (const lxb_char_t *) names[i++];
        size = strlen((const char *) name);

        attr = lxb_dom_element_set_attribute(element, name, size,
                                             (const lxb_char_t *) "oh God", 6);
        test_ne(attr, NULL);

        /* Check exist */
        is_exist = lxb_dom_element_has_attribute(element, name, size);
        test_eq(is_exist, true);

        /* Get value by qualified name */
        value = lxb_dom_element_get_attribute(element, name,
                                              size, &value_len);
        test_ne(value, NULL);

        /* Change value */
        attr = lxb_dom_element_attr_by_name(element, name, size);
        status = lxb_dom_attr_set_value(attr,
                                        (const lxb_char_t *) "new value", 9);

        test_eq(status, LXB_STATUS_OK);

        /* Remove new attribute by name */
        lxb_dom_element_remove_attribute(element, name, size);
    }

    /* Destroy all */
    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(attrs);

    TEST_RUN("lexbor/html/attributes");
    TEST_RELEASE();
}
