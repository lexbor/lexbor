/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/core/core.h>
#include <lexbor/html/html.h>
#include <lexbor/css/css.h>
#include <lexbor/selectors/selectors.h>


typedef struct {
    unsigned          count;
    lexbor_avl_t      *avl;
    lexbor_avl_node_t *root;
}
lxb_example_context_t;


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}

lxb_status_t
find_callback(lxb_dom_node_t *node, lxb_css_selector_specificity_t *spec,
              void *ctx)
{
    lexbor_avl_node_t *avl_node;
    lxb_example_context_t *context = ctx;

    context->count++;

    avl_node = lexbor_avl_search(context->avl, context->root, (uintptr_t) node);
    if (avl_node != NULL) {
        return LXB_STATUS_OK;
    }

    avl_node = lexbor_avl_insert(context->avl, &context->root,
                                 (uintptr_t) node, NULL);
    if (avl_node == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    printf("%u) ", context->count);
    (void) lxb_html_serialize_cb(node, callback, NULL);
    printf("\n");

    return LXB_STATUS_OK;
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_dom_node_t *body;
    lxb_selectors_t *selectors;
    lxb_css_selectors_t *css_selectors;
    lxb_html_document_t *document;
    lxb_css_parser_t *parser;
    lxb_example_context_t ctx;
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

    body = lxb_dom_interface_node(lxb_html_document_body_element(document));

    /* Create CSS parser. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Create CSS Selector parser. */

    css_selectors = lxb_css_selectors_create();
    status = lxb_css_selectors_init(css_selectors, 32);
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

    /* AVL Tree. */

    ctx.avl = lexbor_avl_create();
    status = lexbor_avl_init(ctx.avl, 32);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    ctx.count = 0;
    ctx.root = NULL;

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

    status = lxb_selectors_find(selectors, body, list, find_callback, &ctx);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Destroy AVL Tree with unique nodes. */
    (void) lexbor_avl_destroy(ctx.avl, true);

    /* Destroy Selectors object. */
    (void) lxb_selectors_destroy(selectors, true);

    /* Destroy resources for CSS Parser. */
    (void) lxb_css_parser_destroy(parser, true);

    /* Destroy CSS Selectors List memory. */
    (void) lxb_css_selectors_destroy(css_selectors, true, true);
    /* or use */
    /* lxb_css_selector_list_destroy_memory(list_one); */

    /* Destroy HTML Document. */
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
