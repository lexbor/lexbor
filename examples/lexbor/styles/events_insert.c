/*
 * Copyright (C) 2023-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

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
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *np, *div;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* HTML Data. */

    static const lexbor_str_t html = lexbor_str("<div class=father>"
                                                "<p class=best>a</p>"
                                                "<p>b</p>"
                                                "<s>c</s>"
                                                "</div>");
    /* StyleSheet Data. */

    static const lexbor_str_t slctrs = lexbor_str("div.father {width: 30%}"
                                                  "div.father p.best {width: 20px; height: 10pt}");

    /* Other Data. */

    static const lexbor_str_t father_str = lexbor_str("father");
    static const lexbor_str_t p_str = lexbor_str("p");

    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t best_str = lexbor_str("best");

    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("height: 100px");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    if (document == NULL) {
        return EXIT_FAILURE;
    }

    status = lxb_html_document_parse(document, html.data, html.length);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    printf("HTML:\n");
    status = lxb_html_serialize_tree_cb(lxb_dom_interface_node(document),
                                        callback, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Init all CSS objects and momory for Document. */

    status = lxb_html_document_css_init(document, true);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Create CSS parser. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /*
     * Parse CSS StyleSheet and attach to the Document.
     * At the time of attach StyleSheet, all elements in the Document receive
     * their styles.
     */

    sst = lxb_css_stylesheet_parse(parser, slctrs.data, slctrs.length);
    if (sst == NULL) {
        return EXIT_FAILURE;
    }

    status = lxb_html_document_stylesheet_attach(document, sst);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    printf("\n\nStyleSheet:\n");
    status = lxb_css_rule_serialize(sst->root, callback, NULL);
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

    /* Create new element and append in to div as child. */

    np = lxb_html_document_create_element(document, p_str.data, p_str.length,
                                          NULL);
    if (np == NULL) {
        return EXIT_FAILURE;
    }

    /*
     * Add the attribute class="best" to apply the style from the StyleSheet
     * to the element.
     */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(np),
                                         class_str.data, class_str.length,
                                         best_str.data, best_str.length);
    if (attr == NULL) {
        return EXIT_FAILURE;
    }

    /* Add the attribute style="height: 100px". */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(np),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    if (attr == NULL) {
        return EXIT_FAILURE;
    }

    /*
     * When an element is added to the current tree, all styles
     * will be applied according to the rules of precedence.
     */

    lxb_html_element_insert_child(div, np);

    /* Let's see what we got in the end. */

    printf("\n\nNew element:\n");
    status = lxb_html_serialize_cb(lxb_dom_interface_node(np), callback, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    printf("\n\nHTML after insert new element:\n");
    status = lxb_html_serialize_tree_cb(lxb_dom_interface_node(document),
                                        callback, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    printf("\n\nStyles of new element after insert to tree:\n");

    status = lxb_html_element_style_serialize(np, LXB_DOM_ELEMENT_STYLE_OPT_UNDEF,
                                              callback, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    printf("\n");

    /* Destroy resources. */

    (void) lxb_dom_collection_destroy(collection, true);
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_html_document_css_destroy(document);
    (void) lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
