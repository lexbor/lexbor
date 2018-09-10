/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
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

    /* Get title */
    title = lxb_html_document_title(document, &title_len);
    if (title == NULL) {
        PRINT("Title is empty");
    }
    else {
        PRINT("Title: %s", title);
    }

    /* Get raw title */
    title = lxb_html_document_title_raw(document, &title_len);
    if (title == NULL) {
        PRINT("Raw title is empty");
    }
    else {
        PRINT("Raw title: %s", title);
    }

    /* Destroy document */
    lxb_html_document_destroy(document);

    return 0;
}
