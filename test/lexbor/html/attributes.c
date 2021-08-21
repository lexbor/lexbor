/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>

#include <unit/test.h>


const lxb_char_t html[] = "<div x=abc><span>darkness</span><xx>xXx</xx></div>";

const size_t html_length = sizeof(html) - 1;


TEST_BEGIN(attrs)
{
    lxb_status_t status;
    lxb_html_body_element_t *body;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;
    lxb_dom_element_t *element;
    lxb_dom_attr_t *attr;

    bool is_exist;
    const lxb_char_t *value, *tmp;
    size_t value_len, tmp_len;

    static const lxb_char_t html[] = "<div id=my-best-id></div>";
    size_t html_len = sizeof(html) - 1;

    const lxb_char_t name[] = "id";
    size_t name_size = sizeof(name) - 1;

    /* Parse */
    document = parse(html, html_len);

    /* Print Incoming Data */
    PRINT("HTML:");
    PRINT("%s", (const char *) html);
    PRINT("\nTree after parse:");
    serialize(lxb_dom_interface_node(document));

    /* Create Collection for elements */
    collection = lxb_dom_collection_make(&document->dom_document, 16);
    if (collection == NULL) {
        FAILED("Failed to create collection");
    }

    /* Get BODY elemenet (root for search) */
    body = lxb_html_document_body_element(document);
    element = lxb_dom_interface_element(body);

    /* Find DIV eleemnt */
    status = lxb_dom_elements_by_tag_name(element, collection,
                                          (const lxb_char_t *) "div", 3);

    if (status != LXB_STATUS_OK || lxb_dom_collection_length(collection) == 0) {
        FAILED("Failed to find DIV element");
    }

    /* Append new attrtitube */
    element = lxb_dom_collection_element(collection, 0);

    attr = lxb_dom_element_set_attribute(element, name, name_size,
                                         (const lxb_char_t *) "oh God", 6);
    if (attr == NULL) {
        FAILED("Failed to create and append new attribute");
    }

    /* Print Result */
    PRINT("\nTree after append attribute to DIV element:");
    serialize(lxb_dom_interface_node(document));

    /* Check exist */
    is_exist = lxb_dom_element_has_attribute(element, name, name_size);

    if (is_exist) {
        PRINT("\nElement has attribute \"%s\": true", (const char *) name);
    }
    else {
        PRINT("\nElement has attribute \"%s\": false", (const char *) name);
    }

    /* Get value by qualified name */
    value = lxb_dom_element_get_attribute(element, name, name_size, &value_len);
    if (value == NULL) {
        FAILED("Failed to get attribute value by qualified name");
    }

    PRINT("Get attribute value by qualified name \"%s\": %.*s",
          (const char *) name, (int) value_len, value);

    /* Iterator */
    PRINT("\nGet element attributes by iterator:");
    attr = lxb_dom_element_first_attribute(element);

    while (attr != NULL) {
        tmp = lxb_dom_attr_qualified_name(attr, &tmp_len);
        printf("Name: %s", tmp);

        tmp = lxb_dom_attr_value(attr, &tmp_len);
        if (tmp != NULL) {
            printf("; Value: %s\n", tmp);
        }
        else {
            printf("\n");
        }

        attr = lxb_dom_element_next_attribute(attr);
    }

    /* Change value */
    PRINT("\nChange attribute value:");
    printf("Element before attribute \"%s\" change: ", name);
    serialize_node(lxb_dom_interface_node(element));

    attr = lxb_dom_element_attr_by_name(element, name, name_size);
    status = lxb_dom_attr_set_value(attr, (const lxb_char_t *) "new value", 9);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to change attribute value");
    }

    printf("Element after attribute \"%s\" change: ", name);
    serialize_node(lxb_dom_interface_node(element));

    /* Remove new attrtitube by name */
    lxb_dom_element_remove_attribute(element, name, name_size);

    /* Print Result */
    PRINT("\nTree after remove attribute form DIV element:");
    serialize(lxb_dom_interface_node(document));

    /* Destroy all */
    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(attrs);

    TEST_RUN("lexbor/html/clone");
    TEST_RELEASE();
}
