/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include <string.h>

#include "examples/lexbor/html/base.h"


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
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
    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);

    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create HTML parser");
    }

    /* Parse chunks */
    document = lxb_html_parse_chunk_begin(parser);
    if (document == NULL) {
        FAILED("Failed to create Document object");
    }

    for (size_t i = 0; html[i][0] != '\0'; i++) {
        status = lxb_html_parse_chunk_process(parser, html[i],
                                              strlen((const char *) html[i]));
        if (status != LXB_STATUS_OK) {
            FAILED("Failed to parse HTML chunk");
        }
    }

    status = lxb_html_parse_chunk_end(parser);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse HTML");
    }

    /* Serialization */
    status = lxb_html_serialize_pretty_tree_cb(lxb_dom_interface_node(document),
                                               LXB_HTML_SERIALIZE_OPT_UNDEF,
                                               0, serializer_callback, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to serialization HTML tree");
    }

    /* Destroy */
    lxb_html_document_destroy(document);
    lxb_html_parser_destroy(parser, true);

    return 0;
}
