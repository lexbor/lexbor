/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"

#include <lexbor/dom/dom.h>


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_dom_element_t *element;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    const lxb_char_t html[] = "<div a=b><span></div><div x=z></div>";
    size_t html_szie = sizeof(html) - 1;

    document = parse(html, html_szie);

    collection = lxb_dom_collection_make(&document->dom_document, 128);
    if (collection == NULL) {
        FAILED("Failed to create Collection object");
    }

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document->body),
                                          collection, (const lxb_char_t *) "div", 3);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to get elements by name");
    }

    PRINT("HTML:");
    PRINT("%s", (const char *) html);
    PRINT("\nFind all 'div' elements by tag name 'div'.");
    PRINT("Elements found:");

    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        element = lxb_dom_collection_element(collection, i);

        serialize_node(lxb_dom_interface_node(element));
    }

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);

    return 0;
}
