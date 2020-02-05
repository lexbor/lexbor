/*
 * Copyright (C) 2018-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_html_document_t *document;

    static const lxb_char_t html[][64] = {
        "<!DOCT",
        "YPE htm",
        "l>",
        "<html><head>",
        "<ti",
        "tle>HTML chun",
        "ks parsing</",
        "title>",
        "</head><bod",
        "y><div cla",
        "ss=",
        "\"bestof",
        "class",
        "\">",
        "good for me",
        "</div>",
        "\0"
    };

    /* Initialization */
    document = lxb_html_document_create();
    if (document == NULL) {
        FAILED("Failed to create HTML Document");
    }

    /* Parse HTML */
    status = lxb_html_document_parse_chunk_begin(document);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse HTML");
    }

    PRINT("Incoming HTML chunks:");

    for (size_t i = 0; html[i][0] != '\0'; i++) {
        PRINT("%s", (const char *) html[i]);

        status = lxb_html_document_parse_chunk(document, html[i],
                                               strlen((const char *) html[i]));
        if (status != LXB_STATUS_OK) {
            FAILED("Failed to parse HTML chunk");
        }
    }

    status = lxb_html_document_parse_chunk_end(document);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse HTML");
    }

    /* Print Result */
    PRINT("\nHTML Tree:");
    serialize(lxb_dom_interface_node(document));

    /* Destroy document */
    lxb_html_document_destroy(document);

    return 0;
}
