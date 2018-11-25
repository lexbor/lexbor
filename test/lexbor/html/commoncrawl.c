/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include <unistd.h>

#include <lexbor/html/parser.h>
#include <lexbor/html/serialize.h>
#include <lexbor/html/interfaces/document.h>


lxb_status_t
full_html(void);

lxb_html_document_t *
parse_with_parser(const lxb_char_t *html, size_t html_len);

lxb_status_t
serialize_node(lxb_html_document_t *document);

lxb_inline lxb_status_t
serializer_callback(const lxb_char_t *data, size_t len, void *ctx);


int
main(int argc, const char * argv[])
{
    if (argc == 2) {
        return EXIT_SUCCESS;
    }

    lxb_status_t status;

    status = full_html();
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

lxb_status_t
full_html(void)
{
    lxb_char_t *data;
    ssize_t read_size;
    lxb_html_document_t *document;
    lexbor_str_t str = {0};
    char buf[4096];

    lexbor_mraw_t *mraw = lexbor_mraw_create();
    lxb_status_t status = lexbor_mraw_init(mraw, 4096 * 32);

    if (status != LXB_STATUS_OK) {
        goto done;
    }

    lexbor_str_init(&str, mraw, 4096 * 16);
    if (str.data == NULL) {
        status = LXB_STATUS_ERROR;
        goto done;
    }

    do {
        read_size = read(STDIN_FILENO, &buf, sizeof(buf));

        if (read_size < sizeof(buf) - 1) {
            if (read_size < 0) {
                status = LXB_STATUS_ERROR;
                break;
            }

            data = lexbor_str_append(&str, mraw, 
                                     (lxb_char_t *) buf, (size_t) read_size);
            if (data == NULL) {
                status = LXB_STATUS_ERROR;
                break;
            }

            /* Parse HTML */
            document = parse_with_parser(str.data, str.length);
            if (document == NULL) {
                status = LXB_STATUS_ERROR;
                break;
            }

            status = serialize_node(document);

            lxb_html_document_destroy(document);

            break;
        }

        data = lexbor_str_append(&str, mraw, 
                                 (lxb_char_t *) buf, (size_t) read_size);
        if (data == NULL) {
            status = LXB_STATUS_ERROR;
            break;
        }
    }
    while (1);

done:

    lexbor_mraw_destroy(mraw, true);

    return status;
}

lxb_html_document_t *
parse_with_parser(const lxb_char_t *html, size_t html_len)
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    /* Initialization */
    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);

    if (status != LXB_STATUS_OK) {
        return (void *) lxb_html_parser_destroy(parser, true);
    }

    /* Parse */
    document = lxb_html_parse(parser, html, html_len);
    if (document == NULL) {
        return lxb_html_document_destroy(document);
    }

    /* Destroy parser */
    lxb_html_parser_destroy(parser, true);

    return document;
}

lxb_status_t
serialize_node(lxb_html_document_t *document)
{
    return lxb_html_serialize_pretty_tree_cb(lxb_dom_interface_node(document), 
                                             LXB_HTML_SERIALIZE_OPT_UNDEF,
                                             0, serializer_callback, NULL);
}

lxb_inline lxb_status_t
serializer_callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}
