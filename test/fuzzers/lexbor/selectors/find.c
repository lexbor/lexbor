/*
 * Copyright (C) 2024-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/css/css.h>
#include <lexbor/selectors/selectors.h>


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}

lxb_status_t
find_callback(lxb_dom_node_t *node, lxb_css_selector_specificity_t spec,
              void *ctx)
{
    unsigned *count = ctx;

    (*count)++;

    printf("%u) ", *count);
    (void) lxb_html_serialize_cb(node, callback, NULL);
    printf("\n");

    return LXB_STATUS_OK;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t length)
{
    unsigned count = 0;
    lxb_status_t status;
    lxb_dom_node_t *node;
    lxb_selectors_t *selectors;
    lxb_html_document_t *document;
    lxb_css_parser_t *parser;
    lxb_css_selector_list_t *list;

    /* HTML Data. */

    static const lxb_char_t html[] =     
        "<div div='First' class='Strong Massive'>"
        "    <p p=1><a a=1>a1</a></p>"
        "    <p p=2><a a=2>a2</a></p>"
        "    <p p=3><a a=3>a3</a></p>"
        "    <p p=4><a a=4>a4</a></p>"
        "    <p p=5><a a=5>a5</a></p>"
        "</div>"
        "<div div='Second' class='Massive Stupid'>"
        "<p p=6 lang='en-GB'>"
        "    <span id=s1 span=1><s></s></span>"
        "    <span id=s2 span=2></span>"
        "    <span id=s3 span=3></span>"
        "    <span id=s4 span=4></span>"
        "    <span id=s5 span=5></span>"
        "</p>"
        "<p p=7 lang='ru'>"
        "    <span span=6></span>"
        "    <a a=6><span span=7></span></a>"
        "    <a a=7><span span=8></span></a>"
        "    <span span=9></span>"
        "    <a a=8></a>"
        "    <span span=10></span>"
        "    <span test span=11></span>"
        "    <span test='' span=12></span>"
        "</p>"
        "</div>"
        "<main>"
        "    <h2 h2=1 class=mark></h2>"
        "    <h2 h2=2></h2>"
        "    <h2 h2=3 class=mark></h2>"
        "    <h2 h2=4 class=mark></h2>"
        "    <h2 h2=5></h2>"
        "    <h2 h2=6 class=mark></h2>"
        "</main>";

    /* Create HTML Document. */

    document = lxb_html_document_create();
    status = lxb_html_document_parse(document, html,
                                     sizeof(html) / sizeof(lxb_char_t) - 1);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Create CSS parser. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Selectors. */

    selectors = lxb_selectors_create();
    status = lxb_selectors_init(selectors);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Parse and get the log. */

    list = lxb_css_selectors_parse(parser, data, length);
    if (list == NULL) {
        goto destroy;
    }

    /* Find HTML nodes by CSS Selectors. */

    node = lxb_dom_interface_node(document);

    status = lxb_selectors_find(selectors, node, list, find_callback, &count);
    if (status != LXB_STATUS_OK) {
        printf("Selectors: failed to find: %.*s\n", (int) length, data);
    }

destroy:

    /* Destroy Selectors object. */
    (void) lxb_selectors_destroy(selectors, true);

    /* Destroy resources for CSS Parser. */
    (void) lxb_css_parser_destroy(parser, true);

    /* Destroy all object for all CSS Selector List. */
    lxb_css_selector_list_destroy_memory(list);
    /*
     * for destroy all allocation memory.
     * or use lxb_css_memory_destroy(list->memory, true);
     */

    /* Destroy HTML Document. */
    lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
