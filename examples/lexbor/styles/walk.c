/*
 * Copyright (C) 2023-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/style/style.h>


static lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}

static lxb_status_t
walk_cb(lxb_dom_element_t *element, const lxb_css_rule_declaration_t *declr,
        void *ctx, lxb_css_selector_specificity_t spec, bool is_weak)
{
    lxb_status_t status;

    status = lxb_css_rule_declaration_serialize(declr, callback, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    printf("\n");

    printf("    Name: ");

    status = lxb_css_property_serialize_name(declr->u.user, declr->type,
                                             callback, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    printf("\n    Value: ");

    status = lxb_css_property_serialize(declr->u.user, declr->type,
                                        callback, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    printf("\n    Primary: %s\n", (is_weak) ? "false" : "true");
    printf("    Specificity (priority): %d %d %d %d %d\n",
           lxb_css_selector_sp_i(spec), lxb_css_selector_sp_s(spec),
           lxb_css_selector_sp_a(spec), lxb_css_selector_sp_b(spec),
           lxb_css_selector_sp_c(spec));

    printf("        Important: %d\n", lxb_css_selector_sp_i(spec));
    printf("        From Style Attribute: %d\n", lxb_css_selector_sp_s(spec));
    printf("        A: %d\n", lxb_css_selector_sp_a(spec));
    printf("        B: %d\n", lxb_css_selector_sp_b(spec));
    printf("        C: %d\n", lxb_css_selector_sp_c(spec));

    printf("\n");

    return LXB_STATUS_OK;
}

int
main(int argc, const char * argv[])
{
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* HTML Data. */

    static const lexbor_str_t html = lexbor_str("<div style='width: 321px' class=father></div>");

    /* StyleSheet Data. */

    static const lexbor_str_t slctrs = lexbor_str("div {width: 30%}\n"
                                                  "div.father {width: 20px !important; height: 10pt}");

    /* Other Data. */

    static const lexbor_str_t father_str = lexbor_str("father");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    if (document == NULL) {
        return EXIT_FAILURE;
    }

    /* Init all CSS objects and momory for Document. */

    status = lxb_html_document_css_init(document, true);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Parse HTML. */

    status = lxb_html_document_parse(document, html.data, html.length);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Create CSS parser. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    sst = lxb_css_stylesheet_parse(parser, slctrs.data, slctrs.length);
    if (sst == NULL) {
        return EXIT_FAILURE;
    }

    status = lxb_html_document_stylesheet_attach(document, sst);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Find HTML nodes by CSS Selectors. */

    collection = lxb_dom_collection_make(lxb_dom_interface_document(document),
                                         16);
    if (collection == NULL) {
        return EXIT_FAILURE;
    }

    status = lxb_dom_node_by_class_name(lxb_dom_interface_node(document),
                                        collection, father_str.data,
                                        father_str.length);
    if (status != LXB_STATUS_OK
        || lxb_dom_collection_length(collection) == 0)
    {
        return EXIT_FAILURE;
    }

    div = lxb_html_interface_element(lxb_dom_collection_node(collection, 0));

    /* Input. */

    printf("HTML:\n%.*s\n\n", (int) html.length, (const char *) html.data);
    printf("StyleSheet:\n%.*s\n\n", (int) slctrs.length,
           (const char *) slctrs.data);
    printf("DIV styles assigned:\n");

    /* Walk. */

    status = lxb_dom_element_style_walk(lxb_dom_interface_element(div),
                                        walk_cb, NULL, true);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Destroy resources. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_html_document_css_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
