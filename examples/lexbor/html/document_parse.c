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

    static const lxb_char_t html[] = "<div><p>blah-blah-blah</div>";
    size_t html_len = sizeof(html) - 1;

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

    /* Print Incoming Data */
    PRINT("HTML:");
    PRINT("%s", (const char *) html);

    /* Print Result */
    PRINT("\nHTML Tree:");
    serialize(lxb_dom_interface_node(document));

    /* Destroy document */
    lxb_html_document_destroy(document);

    return 0;
}
