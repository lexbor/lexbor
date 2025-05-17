/*
 * Copyright (C) 2022-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"

#include <lexbor/selectors/selectors.h>
#include <lexbor/style/style.h>


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_dom_element_t *div;
    lxb_html_document_t *doc;
    lxb_css_stylesheet_t *sst;
    lxb_dom_collection_t collection;
    const lxb_css_rule_declaration_t *width, *height;

    static const lexbor_str_t html = lexbor_str(
        "<div style='width: 10px; height: 20pt !important'></div>"
    );

    static const lexbor_str_t css = lexbor_str(
        "div {width: 5pt !important; height: 200px !important}"
    );

    static const lexbor_str_t str_div = lexbor_str("div");
    static const lexbor_str_t str_width = lexbor_str("width");

    /* Create Document. */

    doc = lxb_html_document_create();
    if (doc == NULL) {
        FAILED("Failed to create HTML Document");
    }

    /* Init CSS. */

    status = lxb_html_document_css_init(doc, true);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to CSS initialization");
    }

    /* CSS. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to initialization CSS parser");
    }

    sst = lxb_css_stylesheet_parse(parser, css.data, css.length);
    if (sst == NULL) {
        FAILED("Failed to parse CSS StyleSheet");
    }

    /* HTML. */

    status = lxb_html_document_parse(doc, html.data, html.length);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse HTML");
    }

    /* Attach CSS Styleshet to HTML Document. */

    status = lxb_html_document_stylesheet_attach(doc, sst);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse HTML");
    }

    /* Get <DIV ...>. */

    memset(&collection, 0, sizeof(lxb_dom_collection_t));

    status = lxb_dom_node_by_tag_name(lxb_dom_interface_node(doc), &collection,
                                      str_div.data, str_div.length);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to get element by name");
    }

    div = lxb_dom_collection_element(&collection, 0);
    if (div == NULL) {
        FAILED("Failed to get element by name");
    }

    /* Get style (declaration) from div element. */

    width = lxb_html_element_style_by_name(lxb_html_interface_element(div),
                                           str_width.data, str_width.length);
    if (width == NULL) {
        FAILED("Failed to get style by name");
    }

    height = lxb_html_element_style_by_id(lxb_html_interface_element(div),
                                          LXB_CSS_PROPERTY_HEIGHT);
    if (height == NULL) {
        FAILED("Failed to get style by id");
    }

    /* Serialize. */

    printf("Input HTML:\n%s\n\n", (const char *) html.data);
    printf("Input CSS StyleSheet:\n%s\n\n", (const char *) css.data);
    printf("Get CSS property width and height form div element:\n");

    status = lxb_css_rule_declaration_serialize(width, callback, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to serialize width declaration");
    }

    printf("\n");

    status = lxb_css_rule_declaration_serialize(height, callback, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to serialize height declaration");
    }

    printf("\n");

    (void) lxb_dom_collection_destroy(&collection, false);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_html_document_css_destroy(doc);
    (void) lxb_html_document_destroy(doc);

    return EXIT_SUCCESS;
}
