/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_html_document_t *document;

    const lxb_char_t *title;
    size_t title_len;

    static const lxb_char_t html[] = "<head><title>  Oh,    my...   </title></head>";
    size_t html_len = sizeof(html) - 1;

    const lxb_char_t new_title[] = "We change title";
    size_t new_title_len = sizeof(new_title) - 1;

    /* Initialization */
    document = lxb_html_document_create();
    if (document == NULL) {
        FAILED("Failed to create HTML Document");
    }

    /* Parse HTML */
    status = lxb_html_document_parse(document, html, html_len);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse HTML");
    }

    /* Print HTML tree */
    PRINT("HTML Tree: ");
    serialize(lxb_dom_interface_node(document));

    /* Get title */
    title = lxb_html_document_title(document, &title_len);
    if (title == NULL) {
        PRINT("\nTitle is empty");
    }
    else {
        PRINT("\nTitle: %s", title);
    }

    /* Get raw title */
    title = lxb_html_document_title_raw(document, &title_len);
    if (title == NULL) {
        PRINT("Raw title is empty");
    }
    else {
        PRINT("Raw title: %s", title);
    }

    /* Set new title */
    PRINT("\nChange title to: %s", new_title);

    status = lxb_html_document_title_set(document, new_title, new_title_len);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to change HTML title");
    }

    /* Get new title */
    title = lxb_html_document_title(document, &title_len);
    if (title == NULL) {
        PRINT("New title is empty");
    }
    else {
        PRINT("New title: %s", title);
    }

    /* Print HTML tree after change title */
    PRINT("\nHTML Tree after change title: ");
    serialize(lxb_dom_interface_node(document));

    /* Destroy document */
    lxb_html_document_destroy(document);

    return 0;
}
