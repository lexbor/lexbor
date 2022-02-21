/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"


int
main(int argc, const char *argv[])
{
    lxb_html_element_t *element;
    lxb_html_body_element_t *body;
    lxb_html_document_t *document;

    static const lxb_char_t html[] = "<div><span>blah-blah-blah</div>";
    size_t html_len = sizeof(html) - 1;

    /* try to insert text content */
    static const lxb_char_t inner[] = "<!-- hello world -->";
    size_t inner_len = sizeof(inner) - 1;

    /* Parse */
    document = parse(html, html_len);

    /* Print Incoming Data */
    PRINT("HTML:");
    PRINT("%s", (const char *) html);
    PRINT("\nTree after parse:");
    serialize(lxb_dom_interface_node(document));

    /* Get BODY element */
    body = lxb_html_document_body_element(document);

    PRINT("\nHTML for innerHTML:");
    PRINT("%s", (const char *) inner);

    element = lxb_html_element_inner_html_set(lxb_html_interface_element(body),
                                              inner, inner_len);
    if (element == NULL) {
        FAILED("Failed to parse innerHTML");
    }

    lxb_dom_node_t *child = lxb_dom_interface_node(element)->first_child;
    assert(child);
    // assert(child->type == LXB_DOM_NODE_TYPE_TEXT);
    /* oops, assertion failed */
    assert(lxb_dom_interface_node(element)->owner_document == child->owner_document);

    /* Print Result */
    PRINT("\nTree after innerHTML set:");
    serialize(lxb_dom_interface_node(document));

    /* Destroy all */
    lxb_html_document_destroy(document);

    return 0;
}
