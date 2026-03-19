/*
 * Copyright (C) 2018-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/interfaces/select_element.h"
#include "lexbor/html/interfaces/option_element.h"
#include "lexbor/html/interfaces/document.h"
#include "lexbor/html/interfaces/selectedcontent_element.h"
#include "lexbor/html/common.h"
#include "lexbor/dom/exception.h"


typedef struct {
    lxb_html_option_element_t *first;
    lxb_html_option_element_t *last;
    size_t                    selected;
}
lxb_html_select_selectedness_ctx_t;


static lxb_status_t
lxb_html_select_selectedness_one_cb(lxb_html_select_element_t *el,
                                    lxb_html_option_element_t *option, void *ctx);
static lxb_status_t
lxb_html_select_selectedness_two_cb(lxb_html_select_element_t *el,
                                    lxb_html_option_element_t *option, void *ctx);
static lexbor_action_t
lxb_html_select_find_selectedcontent_cb(lxb_dom_node_t *node, void *ctx);


lxb_html_select_element_t *
lxb_html_select_element_interface_create(lxb_html_document_t *document)
{
    lxb_html_select_element_t *element;

    element = lexbor_mraw_calloc(document->dom_document.mraw,
                                 sizeof(lxb_html_select_element_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = lxb_html_document_original_ref(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    return element;
}

lxb_html_select_element_t *
lxb_html_select_element_interface_destroy(lxb_html_select_element_t *select_element)
{
    (void) lxb_dom_node_interface_destroy(lxb_dom_interface_node(select_element));
    return NULL;
}

size_t
lxb_html_select_selectedness_display_size(lxb_html_select_element_t *el)
{
    int64_t len;
    lexbor_str_t *value;
    lxb_status_t status;
    lxb_dom_attr_t *size, *multiple;

    size = lxb_dom_element_attr_by_id(lxb_dom_interface_element(el),
                                      LXB_DOM_ATTR_SIZE);
    if (size == NULL) {
        goto default_size;
    }

    value = size->value;

    status = lxb_html_common_parsing_nonneg_integer(value->data, value->length,
                                                    &len);
    if (status != LXB_STATUS_OK || len < 0) {
        goto default_size;
    }

    return (size_t) len;

default_size:

    multiple = lxb_dom_element_attr_by_id(lxb_dom_interface_element(el),
                                          LXB_DOM_ATTR_MULTIPLE);
    if (multiple == NULL) {
        return 1;
    }

    return 4;
}

lxb_dom_exception_code_t
lxb_html_select_selectedness_setting_algorithm(lxb_html_select_element_t *el)
{
    lxb_status_t status;
    size_t display_size;
    lxb_dom_attr_t *multiple;
    lxb_html_select_selectedness_ctx_t snctx;

    multiple = lxb_dom_element_attr_by_id(lxb_dom_interface_element(el),
                                           LXB_DOM_ATTR_MULTIPLE);
    if (multiple != NULL) {
        return LXB_DOM_EXCEPTION_OK;
    }

    /*
     * Step 1. If the element's multiple attribute is absent, and the element's
     * display size is 1, and no option elements in the element's list of
     * options have their selectedness set to true, then set the selectedness
     * of the first option element in the list of options in tree order that is
     * not disabled, if any, to true and return.
     */

    display_size = lxb_html_select_selectedness_display_size(el);

    if (display_size == 1) {
        snctx.first = NULL;
        snctx.last = NULL;
        snctx.selected = 0;

        status = lxb_html_select_list_of_options(el,
                                                 lxb_html_select_selectedness_one_cb,
                                                 &snctx);
        if (status != LXB_STATUS_OK) {
            return LXB_DOM_EXCEPTION_ERR;
        }

        if (snctx.selected == 0) {
            if (snctx.first != NULL) {
                snctx.first->selectedness = true;
            }

            return LXB_DOM_EXCEPTION_OK;
        }
    }

    /*
     * Step 2. If the element's multiple attribute is absent, and two or more
     * option elements in the element's list of options have their selectedness
     * set to true, then set the selectedness of all but the last option element
     * with its selectedness set to true in the list of options in tree order
     * to false.
     */

    if (snctx.selected >= 2) {
        status = lxb_html_select_list_of_options(el,
                                                 lxb_html_select_selectedness_two_cb,
                                                 &snctx);
        if (status != LXB_STATUS_OK) {
            return LXB_DOM_EXCEPTION_ERR;
        }
    }

    return LXB_DOM_EXCEPTION_OK;
}

static lxb_status_t
lxb_html_select_selectedness_one_cb(lxb_html_select_element_t *el,
                                    lxb_html_option_element_t *option, void *ctx)
{
    lxb_html_select_selectedness_ctx_t *snctx = ctx;

    if (option->selectedness) {
        snctx->selected += 1;
        snctx->last = option;
    }

    if (snctx->first == NULL
        && !lxb_html_option_is_disabled(option))
    {
        snctx->first = option;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_select_selectedness_two_cb(lxb_html_select_element_t *el,
                                    lxb_html_option_element_t *option, void *ctx)
{
    lxb_html_select_selectedness_ctx_t *snctx = ctx;

    if (option->selectedness && option != snctx->last) {
        option->selectedness = false;
    }

    return LXB_STATUS_OK;
}

lxb_html_selectedcontent_element_t *
lxb_html_select_get_enabled_selectedcontent(lxb_html_select_element_t *el)
{
    lxb_dom_attr_t *multiple;
    lxb_dom_node_t *selectedcontent, *node;
    lxb_html_selectedcontent_element_t *sc;

    multiple = lxb_dom_element_attr_by_id(lxb_dom_interface_element(el),
                                          LXB_DOM_ATTR_MULTIPLE);
    if (multiple != NULL) {
        return NULL;
    }

    selectedcontent = NULL;
    node = lxb_dom_interface_node(el);

    lxb_dom_node_simple_walk(node, lxb_html_select_find_selectedcontent_cb,
                             &selectedcontent);
    if (selectedcontent == NULL) {
        return NULL;
    }

    sc = lxb_html_interface_selectedcontent(selectedcontent);

    return sc->disabled ? NULL : sc;
}

static lexbor_action_t
lxb_html_select_find_selectedcontent_cb(lxb_dom_node_t *node, void *ctx)
{
    lxb_dom_node_t **selectedcontent;

    if (node->local_name == LXB_TAG_SELECTEDCONTENT
        && node->ns == LXB_NS_HTML)
    {
        selectedcontent = ctx;
        *selectedcontent = node;

        return LEXBOR_ACTION_STOP;
    }

    return LEXBOR_ACTION_OK;
}

lxb_status_t
lxb_html_select_element_insert_steps(lxb_dom_node_t *inserted_node)
{
    lxb_html_select_element_t *select = lxb_html_interface_select(inserted_node);

    (void) lxb_html_select_selectedness_setting_algorithm(select);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_select_list_of_options(lxb_html_select_element_t *el,
                                lxb_html_select_options_cb_f walker_cb, void *ctx)
{
    bool skip;
    lxb_status_t status;
    lxb_dom_node_t *node, *root, *optgroup;

    root = lxb_dom_interface_node(el);
    node = root->first_child;

    while (node != NULL) {
        if (node->local_name == LXB_TAG_OPTION && node->ns == LXB_NS_HTML) {
            status = walker_cb(el, lxb_html_interface_option(node), ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }
        }

        skip = false;

        if (node->ns == LXB_NS_HTML) {
            switch (node->local_name) {
                case LXB_TAG_SELECT:
                case LXB_TAG_DATALIST:
                case LXB_TAG_HR:
                case LXB_TAG_OPTION:
                    skip = true;
                    break;

                case LXB_TAG_OPTGROUP:
                    optgroup = node->parent;

                    while (optgroup != root) {
                        if (optgroup->local_name != LXB_TAG_OPTGROUP
                            && optgroup->ns == LXB_NS_HTML)
                        {
                            skip = true;
                            break;
                        }

                        optgroup = optgroup->parent;
                    }

                    break;

                default:
                    break;
            }
        }

        if (node->first_child != NULL && skip) {
            node = node->first_child;
        }
        else {
            while(node != root && node->next == NULL) {
                node = node->parent;
            }

            if (node == root) {
                break;
            }

            node = node->next;
        }
    }

    return LXB_STATUS_OK;
}
