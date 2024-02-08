/*
 * Copyright (C) 2021 Alexander Borisov
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

    *count += 1;

    printf("%u) ", *count);
    (void) lxb_html_serialize_cb(node, callback, NULL);
    printf("\n");

    return LXB_STATUS_OK;
}

int
main(int argc, const char *argv[])
{
    unsigned count;
    lxb_status_t status;
    lxb_dom_node_t *body;
    lxb_selectors_t *selectors;
    lxb_css_selectors_t *css_selectors;
    lxb_html_document_t *document;
    lxb_css_parser_t *parser;
    lxb_css_selector_list_t *list;

    /* HTML Data. */

    static const lxb_char_t html[] = "<div><p class='x z'> </p><p id='y'>abc</p></div>";

    /* CSS Data. */

    static const lxb_char_t slctrs[] = ".x, div:has(p[id=Y i]), p.x, p:blank, div";

    /* Create HTML Document. */

    document = lxb_html_document_create();
    status = lxb_html_document_parse(document, html,
                                     sizeof(html) / sizeof(lxb_char_t) - 1);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    body = lxb_dom_interface_node(document);

    /* Create CSS parser. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Create CSS Selector parser. */

    css_selectors = lxb_css_selectors_create();
    status = lxb_css_selectors_init(css_selectors);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* It is important that a new selector object is not created internally
     * for each call to the parser.
     */
    lxb_css_parser_selectors_set(parser, css_selectors);

    /* Selectors. */

    selectors = lxb_selectors_create();
    status = lxb_selectors_init(selectors);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Parse and get the log. */

    list = lxb_css_selectors_parse(parser, slctrs,
                                   (sizeof(slctrs) / sizeof(lxb_char_t)) - 1);
    if (list == NULL) {
        return EXIT_FAILURE;
    }

    /* HTML serialization. */

    printf("HTML:\n");
    (void) lxb_html_serialize_pretty_deep_cb(body, 0, 0, callback, NULL);
    printf("\n");

    /* Selector List Serialization. */

    printf("Selectors: ");
    (void) lxb_css_selector_serialize_list_chain(list, callback, NULL);
    printf("\n");

    /* Find HTML nodes by CSS Selectors. */

    printf("\nFound:\n");

    count = 0;

    lxb_selectors_opt_set(selectors, LXB_SELECTORS_OPT_MATCH_FIRST);

    status = lxb_selectors_find(selectors, body, list, find_callback, &count);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Destroy Objects. */

    (void) lxb_selectors_destroy(selectors, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_css_memory_destroy(list->memory, true);

    /*
     * or use lxb_css_selector_list_destroy_memory(list);
     */

    (void) lxb_css_selectors_destroy(css_selectors, true);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
