/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/attribute_steps.h"
#include "lexbor/html/interfaces/option_element.h"
#include "lexbor/dom/interfaces/document.h"
#include "lexbor/dom/interfaces/element.h"

#include "lexbor/tag/const.h"
#include "lexbor/ns/const.h"
#include "lexbor/html/attribute_steps_res.h"


lxb_status_t
lxb_html_attribute_steps_change(lxb_dom_element_t *element,
                                lxb_dom_attr_id_t name,
                                const lxb_char_t *old_value, size_t old_len,
                                const lxb_char_t *value, size_t value_len,
                                lxb_ns_id_t ns)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_t *node;
    lxb_dom_element_attr_change_f change;

    node = lxb_dom_interface_node(element);
    tag_id = node->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_attr_change(element, name, old_value,
                                            old_len, value, value_len, ns);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    change = lxb_html_attribute_steps_res_default[element->node.local_name].change;

    if (change != NULL) {
        return change(element, name, old_value, old_len, value, value_len, ns);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_attribute_steps_append(lxb_dom_element_t *element,
                                lxb_dom_attr_id_t name,
                                const lxb_char_t *old_value, size_t old_len,
                                const lxb_char_t *value, size_t value_len,
                                lxb_ns_id_t ns)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_t *node;
    lxb_dom_element_attr_change_f append;

    node = lxb_dom_interface_node(element);
    tag_id = node->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_attr_append(element, name, old_value,
                                            old_len, value, value_len, ns);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    append = lxb_html_attribute_steps_res_default[element->node.local_name].append;

    if (append != NULL) {
        return append(element, name, old_value, old_len, value, value_len, ns);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_attribute_steps_remove(lxb_dom_element_t *element,
                                lxb_dom_attr_id_t name,
                                const lxb_char_t *old_value, size_t old_len,
                                const lxb_char_t *value, size_t value_len,
                                lxb_ns_id_t ns)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_t *node;
    lxb_dom_element_attr_change_f remove;

    node = lxb_dom_interface_node(element);
    tag_id = node->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_attr_remove(element, name, old_value,
                                            old_len, value, value_len, ns);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    remove = lxb_html_attribute_steps_res_default[element->node.local_name].remove;

    if (remove != NULL) {
        return remove(element, name, old_value, old_len, value, value_len, ns);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_attribute_steps_replace(lxb_dom_element_t *element,
                                 lxb_dom_attr_id_t name,
                                 const lxb_char_t *old_value, size_t old_len,
                                 const lxb_char_t *value, size_t value_len,
                                 lxb_ns_id_t ns)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_t *node;
    lxb_dom_element_attr_change_f replace;

    node = lxb_dom_interface_node(element);
    tag_id = node->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_attr_replace(element, name, old_value,
                                             old_len, value, value_len, ns);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    replace = lxb_html_attribute_steps_res_default[element->node.local_name].remove;

    if (replace != NULL) {
        return replace(element, name, old_value, old_len, value, value_len, ns);
    }

    return LXB_STATUS_OK;
}
