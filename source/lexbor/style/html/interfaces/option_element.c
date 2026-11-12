/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/html/interfaces/option_element.h"
#include "lexbor/html/interfaces/option_element.h"
#include "lexbor/style/style.h"


/*
 * Element steps.
 */
lxb_status_t
lxb_style_html_option_inserted_steps(lxb_dom_node_t *inserted_node)
{
    lxb_status_t status;

    if (inserted_node->ns == LXB_NS_HTML) {
        status = lxb_html_option_element_insert_steps(inserted_node);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_style_html_element_inserted_steps(inserted_node);
}

lxb_status_t
lxb_style_html_option_removed_steps(lxb_dom_node_t *removed_node,
                                    lxb_dom_node_t *old_parent)
{
    lxb_status_t status;

    if (removed_node->ns == LXB_NS_HTML) {
        status = lxb_html_option_element_remove_steps(removed_node, old_parent);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_style_html_element_removed_steps(removed_node, old_parent);
}

/*
 * Attribute steps.
 */
lxb_status_t
lxb_style_html_option_attr_change(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns)
{
    lxb_status_t status;

    if (element->node.ns == LXB_NS_HTML) {
        status = lxb_html_option_attr_steps_change(element, name,
                                                   old_value, old_len,
                                                   value, value_len, ns);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_style_html_element_attr_change(element, name, old_value,
                                              old_len, value, value_len, ns);
}

lxb_status_t
lxb_style_html_option_attr_append(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns)
{
    lxb_status_t status;

    if (element->node.ns == LXB_NS_HTML) {
        status = lxb_html_option_attr_steps_change(element, name,
                                                   old_value, old_len,
                                                   value, value_len, ns);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_style_html_element_attr_append(element, name, old_value,
                                              old_len, value, value_len, ns);
}

lxb_status_t
lxb_style_html_option_attr_remove(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns)
{
    lxb_status_t status;

    if (element->node.ns == LXB_NS_HTML) {
        status = lxb_html_option_attr_steps_remove(element, name,
                                                   old_value, old_len,
                                                   value, value_len, ns);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_style_html_element_attr_remove(element, name, old_value,
                                              old_len, value, value_len, ns);
}

lxb_status_t
lxb_style_html_option_attr_replace(lxb_dom_element_t *element,
                                   lxb_dom_attr_id_t name,
                                   const lxb_char_t *old_value, size_t old_len,
                                   const lxb_char_t *value, size_t value_len,
                                   lxb_ns_id_t ns)
{
    return lxb_style_html_element_attr_replace(element, name, old_value,
                                               old_len, value, value_len, ns);
}
