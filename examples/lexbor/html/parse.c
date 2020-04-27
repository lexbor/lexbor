/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
    lxb_html_document_t *doc_one, *doc_two;

    static const lxb_char_t html_one[] = "<div><p>First</div>";
    size_t html_one_len = sizeof(html_one) - 1;

    static const lxb_char_t html_two[] = "<div><p>Second</div>";
    size_t html_two_len = sizeof(html_two) - 1;

    /* Initialization */
    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);

    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create HTML parser");
    }

    /* Parse */
    doc_one = lxb_html_parse(parser, html_one, html_one_len);
    if (doc_one == NULL) {
        FAILED("Failed to create Document object");
    }

    doc_two = lxb_html_parse(parser, html_two, html_two_len);
    if (doc_two == NULL) {
        FAILED("Failed to create Document object");
    }

    /* Destroy parser */
    lxb_html_parser_destroy(parser);

    /* Serialization */
    printf("First Document:\n");

    status = lxb_html_serialize_pretty_tree_cb(lxb_dom_interface_node(doc_one),
                                               LXB_HTML_SERIALIZE_OPT_UNDEF,
                                               0, serializer_callback, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to serialization HTML tree");
    }

    printf("\nSecond Document:\n");

    status = lxb_html_serialize_pretty_tree_cb(lxb_dom_interface_node(doc_two),
                                               LXB_HTML_SERIALIZE_OPT_UNDEF,
                                               0, serializer_callback, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to serialization HTML tree");
    }

    /* Destroy */
    lxb_html_document_destroy(doc_one);
    lxb_html_document_destroy(doc_two);

    return 0;
}
