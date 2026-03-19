/*
 * Copyright (C) 2018-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/interfaces/option_element.h"
#include "lexbor/html/interfaces/select_element.h"
#include "lexbor/html/interfaces/selectedcontent_element.h"
#include "lexbor/html/interfaces/document.h"


static lxb_html_select_element_t *
lxb_html_option_element_nearest_ancestor_select(lxb_html_option_element_t *option);

lxb_html_option_element_t *
lxb_html_option_element_interface_create(lxb_html_document_t *document)
{
    lxb_html_option_element_t *element;

    element = lexbor_mraw_calloc(document->dom_document.mraw,
                                 sizeof(lxb_html_option_element_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    /* selectedness default is false. */

    node->owner_document = lxb_html_document_original_ref(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    return element;
}

lxb_html_option_element_t *
lxb_html_option_element_interface_destroy(lxb_html_option_element_t *option_element)
{
    (void) lxb_dom_node_interface_destroy(lxb_dom_interface_node(option_element));
    return NULL;
}

lxb_dom_exception_code_t
lxb_html_option_element_insertion(lxb_html_option_element_t *option)
{
    lxb_html_select_element_t *select;

    select = lxb_html_option_element_nearest_ancestor_select(option);
    if (select == NULL) {
        return LXB_DOM_EXCEPTION_OK;
    }

    return LXB_DOM_EXCEPTION_OK;
}

lxb_dom_exception_code_t
lxb_html_option_maybe_clone_to_selectedcontent(lxb_html_option_element_t *option)
{
    bool is;
    lxb_html_select_element_t *select;
    lxb_html_selectedcontent_element_t *sel_content;

    is = lxb_html_option_element_selectedness(option);
    if (!is) {
        return LXB_DOM_EXCEPTION_OK;
    }

    select = lxb_html_option_element_nearest_ancestor_select(option);
    if (select == NULL) {
        return LXB_DOM_EXCEPTION_OK;
    }

    sel_content = lxb_html_select_get_enabled_selectedcontent(select);
    if (sel_content == NULL) {
        return LXB_DOM_EXCEPTION_OK;
    }

    return lxb_html_selectedcontent_clone_option(sel_content, option);
}

bool
lxb_html_option_element_selectedness(lxb_html_option_element_t *option)
{
    return option->selectedness;
}

static lxb_html_select_element_t *
lxb_html_option_element_nearest_ancestor_select(lxb_html_option_element_t *option)
{
    lxb_dom_node_t *node;
    lxb_dom_node_t *optgroup = NULL;

    node = lxb_dom_interface_node(option)->parent;

    while (node != NULL) {
        switch (node->local_name) {
            case LXB_TAG_DATALIST:
            case LXB_TAG_HR:
            case LXB_TAG_OPTION:
                if (node->ns == LXB_NS_HTML) {
                    return NULL;
                }

                break;

            case LXB_TAG_OPTGROUP:
                if (optgroup != NULL && node->ns == LXB_NS_HTML) {
                    return NULL;
                }

                if (node->ns == LXB_NS_HTML) {
                    optgroup = node;
                }

                break;

            case LXB_TAG_SELECT:
                if (node->ns == LXB_NS_HTML) {
                    return lxb_html_interface_select(node);
                }

                break;

            default:
                break;
        }

        node = node->parent;
    }

    return NULL;
}

bool
lxb_html_option_is_disabled(lxb_html_option_element_t *option)
{
    lxb_dom_attr_t *attr;
    lxb_dom_node_t *node = lxb_dom_interface_node(option);

    attr = lxb_dom_element_attr_by_id(lxb_dom_interface_element(node),
                                      LXB_DOM_ATTR_DISABLED);
    if (attr != NULL) {
        return true;
    }

    node = node->parent;

    while (node != NULL) {
        switch (node->local_name) {
            case LXB_TAG_SELECT:
            case LXB_TAG_DATALIST:
            case LXB_TAG_HR:
            case LXB_TAG_OPTION:
                if (node->ns == LXB_NS_HTML) {
                    return false;
                }

                break;

            case LXB_TAG_OPTGROUP:
                if (node->ns == LXB_NS_HTML) {
                    attr = lxb_dom_element_attr_by_id(lxb_dom_interface_element(node),
                                                      LXB_DOM_ATTR_DISABLED);
                    return (attr != NULL);
                }

                break;

            default:
                break;
        }

        node = node->parent;
    }

    return false;
}

lxb_dom_exception_code_t
lxb_html_option_update_nearest_ancestor_select(lxb_html_option_element_t *option)
{
    lxb_html_select_element_t *sel;

    sel = lxb_html_option_element_nearest_ancestor_select(option);
    if (sel == NULL) {
        return LXB_DOM_EXCEPTION_OK;
    }

    return lxb_html_select_selectedness_setting_algorithm(sel);
}

lxb_status_t
lxb_html_option_element_pop_open_elements(lxb_dom_node_t *node)
{
    lxb_dom_exception_code_t code;
    lxb_html_option_element_t *option;

    option = lxb_html_interface_option(node);
    code = lxb_html_option_maybe_clone_to_selectedcontent(option);

    return (code == LXB_DOM_EXCEPTION_OK) ? LXB_STATUS_OK : LXB_STATUS_ERROR;
}

lxb_status_t
lxb_html_option_element_insert_steps(lxb_dom_node_t *inserted_node)
{
    lxb_html_option_element_t *option;

    option = lxb_html_interface_option(inserted_node);

    (void) lxb_html_option_update_nearest_ancestor_select(option);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_option_element_remove_steps(lxb_dom_node_t *removed_node,
                                     lxb_dom_node_t *old_parent)
{
    lxb_html_option_element_t *option;

    option = lxb_html_interface_option(removed_node);

    (void) lxb_html_option_update_nearest_ancestor_select(option);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_option_attr_steps_change(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns)
{
    lxb_html_option_element_t *option = lxb_html_interface_option(element);

    if (name == LXB_DOM_ATTR_SELECTED) {
        option->selectedness = true;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_option_attr_steps_remove(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns)
{
    lxb_html_option_element_t *option = lxb_html_interface_option(element);

    if (name == LXB_DOM_ATTR_SELECTED) {
        option->selectedness = false;
    }

    return LXB_STATUS_OK;
}
