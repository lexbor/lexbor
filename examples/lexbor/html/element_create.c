/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"


int
main(int argc, const char *argv[])
{
    lxb_tag_id_t tag_id;
    lxb_html_body_element_t *body;
    lxb_html_document_t *document;
    lxb_dom_element_t *element;
    lxb_dom_text_t *text;
    lexbor_hash_t *tags;

    const lxb_char_t *tag_name;
    size_t tag_name_len;

    /* Parse */
    document = parse((const lxb_char_t *) "", 0);

    body = lxb_html_document_body_element(document);

    /* Print Current Tree */
    PRINT("Inital HTML Tree:");
    serialize(lxb_dom_interface_node(document));
    printf("\n");

    /* Create all known tags */
    tags = lxb_html_document_tags(document);

    for (tag_id = LXB_TAG_A; tag_id < LXB_TAG__LAST_ENTRY; tag_id++)
    {
        tag_name = lxb_tag_name_by_id(tags, tag_id, &tag_name_len);
        if (tag_name == NULL) {
            FAILED("Failed to get tag name by id");
        }

        element = lxb_dom_document_create_element(&document->dom_document,
                                                  tag_name, tag_name_len, NULL);
        if (element == NULL) {
            FAILED("Failed to create element for tag \"%s\"",
                   (const char *) tag_name);
        }

        /*
         * If the tag is void then we do not create a text node.
         * See https://html.spec.whatwg.org/multipage/syntax.html#void-elements
         */
        if (lxb_html_tag_is_void(tag_id)) {
            printf("Create element by tag name \"%s\": ",
                   (const char *) tag_name);
        }
        else {
            printf("Create element by tag name \"%s\" and append text node: ",
                   (const char *) tag_name);

            text = lxb_dom_document_create_text_node(&document->dom_document,
                                                     tag_name, tag_name_len);
            if (text == NULL) {
                FAILED("Failed to create text node for \"%s\"",
                       (const char *) tag_name);
            }

            lxb_dom_node_insert_child(lxb_dom_interface_node(element),
                                      lxb_dom_interface_node(text));
        }

        serialize_node(lxb_dom_interface_node(element));

        lxb_dom_node_insert_child(lxb_dom_interface_node(body),
                                  lxb_dom_interface_node(element));
    }

    /* Print Result */
    PRINT("\nTree after create elements:");
    serialize(lxb_dom_interface_node(document));

    /* Destroy all */
    lxb_html_document_destroy(document);

    return 0;
}
