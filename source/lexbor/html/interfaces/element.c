/*
 * Copyright (C) 2018-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/interfaces/element.h"
#include "lexbor/html/interfaces/document.h"


lxb_html_element_t *
lxb_html_element_interface_create(lxb_html_document_t *document)
{
    lxb_html_element_t *element;

    element = lexbor_mraw_calloc(document->dom_document.mraw,
                                 sizeof(lxb_html_element_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = lxb_html_document_original_ref(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    return element;
}

lxb_html_element_t *
lxb_html_element_interface_destroy(lxb_html_element_t *element)
{
    (void) lxb_dom_node_interface_destroy(lxb_dom_interface_node(element));
    return NULL;
}

lxb_html_element_t *
lxb_html_element_inner_html_set(lxb_html_element_t *element,
                                const lxb_char_t *html, size_t size)
{
    lxb_dom_node_t *node, *child;
    lxb_dom_node_t *root = lxb_dom_interface_node(element);
    lxb_html_document_t *doc = lxb_html_interface_document(root->owner_document);

    node = lxb_html_document_parse_fragment(doc, &element->element, html, size);
    if (node == NULL) {
        return NULL;
    }

    while (root->first_child != NULL) {
        lxb_dom_node_destroy_deep(root->first_child);
    }

    while (node->first_child != NULL) {
        child = node->first_child;

        lxb_dom_node_remove(child);
        lxb_dom_node_insert_child(root, child);
    }

    lxb_dom_node_destroy(node);

    return lxb_html_interface_element(root);
}

/*
 * Unknown Element steps.
 */
lxb_status_t
lxb_html_element_inserted_unknown_steps(lxb_dom_node_t *inserted_node)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_removed_unknown_steps(lxb_dom_node_t *removed_node,
                                             lxb_dom_node_t *old_parent)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_moved_unknown_steps(lxb_dom_node_t *moved_node,
                                           lxb_dom_node_t *old_parent)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_destroy_unknown_steps(lxb_dom_node_t *node)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_children_changed_unknown_steps(lxb_dom_node_t *parent)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_connected_unknown_steps(lxb_dom_node_t *connected_node)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_attr_change(lxb_dom_element_t *element,
                             lxb_dom_attr_id_t local_name,
                             const lxb_char_t *old_value, size_t old_len,
                             const lxb_char_t *value, size_t value_len,
                             lxb_ns_id_t ns)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_attr_append(lxb_dom_element_t *element,
                             lxb_dom_attr_id_t local_name,
                             const lxb_char_t *old_value, size_t old_len,
                             const lxb_char_t *value, size_t value_len,
                             lxb_ns_id_t ns)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_attr_remove(lxb_dom_element_t *element,
                             lxb_dom_attr_id_t local_name,
                             const lxb_char_t *old_value, size_t old_len,
                             const lxb_char_t *value, size_t value_len,
                             lxb_ns_id_t ns)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_attr_replace(lxb_dom_element_t *element,
                              lxb_dom_attr_id_t local_name,
                              const lxb_char_t *old_value, size_t old_len,
                              const lxb_char_t *value, size_t value_len,
                              lxb_ns_id_t ns)
{
    return LXB_STATUS_OK;
}
