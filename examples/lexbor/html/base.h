/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_EXAMPLES_BASE_H
#define LEXBOR_EXAMPLES_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/html.h"


#define FAILED(...)                                                            \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    while (0)

#define PRINT(...)                                                             \
    do {                                                                       \
        fprintf(stdout, __VA_ARGS__);                                          \
        fprintf(stdout, "\n");                                                 \
    }                                                                          \
    while (0)


lxb_inline lxb_status_t
serializer_callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}

lxb_inline lxb_html_document_t *
parse(const lxb_char_t *html, size_t html_len)
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    /* Initialization */
    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);

    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create HTML parser");
    }

    /* Parse */
    document = lxb_html_parse(parser, html, html_len);
    if (document == NULL) {
        FAILED("Failed to create Document object");
    }

    /* Destroy parser */
    lxb_html_parser_destroy(parser);

    return document;
}

lxb_inline void
serialize(lxb_dom_node_t *node)
{
    lxb_status_t status;

    status = lxb_html_serialize_pretty_tree_cb(node,
                                               LXB_HTML_SERIALIZE_OPT_UNDEF,
                                               0, serializer_callback, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to serialization HTML tree");
    }
}

lxb_inline void
serialize_node(lxb_dom_node_t *node)
{
    lxb_status_t status;

    status = lxb_html_serialize_pretty_cb(node, LXB_HTML_SERIALIZE_OPT_UNDEF,
                                          0, serializer_callback, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to serialization HTML tree");
    }
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_EXAMPLES_BASE_H */
