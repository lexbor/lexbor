/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"

#include <lexbor/dom/dom.h>


static void
print_collection_elements(lxb_dom_collection_t *collection)
{
    lxb_dom_element_t *element;

    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        element = lxb_dom_collection_element(collection, i);

        serialize_node(lxb_dom_interface_node(element));
    }

    lxb_dom_collection_clean(collection);
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_dom_element_t *body;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    const lxb_char_t html[] = "<div class=\"best blue some\"><span></div>"
    "<div class=\"red pref_best grep\"></div>"
    "<div class=\"green best grep\"></div>"
    "<a href=\"http://some.link/\">ref</a>"
    "<div class=\"red c++ best\"></div>";

    size_t html_szie = sizeof(html) - 1;

    PRINT("HTML:");
    PRINT("%s", (const char *) html);

    document = parse(html, html_szie);

    body = lxb_dom_interface_element(document->body);

    collection = lxb_dom_collection_make(&document->dom_document, 128);
    if (collection == NULL) {
        FAILED("Failed to create Collection object");
    }

    /* Full match */
    status = lxb_dom_elements_by_attr(body, collection,
                                      (const lxb_char_t *) "class", 5,
                                      (const lxb_char_t *) "red c++ best", 12,
                                      true);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to get elements by name");
    }

    PRINT("\nFull match by 'red c++ best':");
    print_collection_elements(collection);

    /* From begin */
    status = lxb_dom_elements_by_attr_begin(body, collection,
                                            (const lxb_char_t *) "href", 4,
                                            (const lxb_char_t *) "http", 4,
                                            true);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to get elements by name");
    }

    PRINT("\nFrom begin by 'http':");
    print_collection_elements(collection);

    /* From end */
    status = lxb_dom_elements_by_attr_end(body, collection,
                                          (const lxb_char_t *) "class", 5,
                                          (const lxb_char_t *) "grep", 4,
                                          true);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to get elements by name");
    }

    PRINT("\nFrom end by 'grep':");
    print_collection_elements(collection);

    /* Contain */
    status = lxb_dom_elements_by_attr_contain(body, collection,
                                              (const lxb_char_t *) "class", 5,
                                              (const lxb_char_t *) "c++ b", 5,
                                              true);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to get elements by name");
    }

    PRINT("\nContain by 'c++ b':");
    print_collection_elements(collection);

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);

    return 0;
}
