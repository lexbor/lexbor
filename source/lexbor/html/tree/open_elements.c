/*
 * Copyright (C) 2018-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tree/open_elements.h"
#include "lexbor/html/tree.h"

#include "lexbor/html/interfaces/option_element.h"
#include "lexbor/html/tree/open_elements_res.h"


lxb_inline lxb_status_t
lxb_html_tree_open_elements_pop_cb(lxb_dom_node_t *node)
{
    lxb_html_document_open_elements_pop_f pop_cb;

    if (node->local_name > LXB_TAG__TEXT
        && node->local_name < LXB_TAG__LAST_ENTRY
        && node->ns == LXB_NS_HTML)
    {
        pop_cb = lxb_html_interface_document(node->owner_document)->open_pop[node->local_name];

        if (pop_cb != NULL) {
            return pop_cb(node);
        }
    }

    return LXB_STATUS_OK;
}

LXB_API void
lxb_html_tree_open_elements_remove_by_node(lxb_html_tree_t *tree,
                                           lxb_dom_node_t *node)
{
    size_t delta;
    void **list = tree->open_elements->list;
    size_t len = tree->open_elements->length;

    while (len != 0) {
        len--;

        if (list[len] == node) {
            delta = tree->open_elements->length - len - 1;

            memmove(list + len, list + len + 1, sizeof(void *) * delta);

            tree->open_elements->length--;

            break;
        }
    }
}

lxb_status_t
lxb_html_tree_open_elements_pop_until_tag_id(lxb_html_tree_t *tree,
                                             lxb_tag_id_t tag_id,
                                             lxb_ns_id_t ns,
                                             bool exclude)
{
    void **list;
    unsigned status;
    lxb_dom_node_t *node;

    list = tree->open_elements->list;
    status = LXB_STATUS_OK;

    while (tree->open_elements->length != 0) {
        tree->open_elements->length--;

        node = list[ tree->open_elements->length ];

        if (node->local_name == tag_id && node->ns == ns) {
            if (exclude == false) {
                tree->open_elements->length++;
            }
            else {
                status |= (unsigned) lxb_html_tree_open_elements_pop_cb(node);
            }

            break;
        }

        status |= (unsigned) lxb_html_tree_open_elements_pop_cb(node);
    }

    return (status == LXB_STATUS_OK) ? LXB_STATUS_OK : LXB_STATUS_ERROR;
}

lxb_status_t
lxb_html_tree_open_elements_pop_until_h123456(lxb_html_tree_t *tree)
{
    void **list;
    unsigned status;
    lxb_dom_node_t *node;

    list = tree->open_elements->list;
    status = LXB_STATUS_OK;

    while (tree->open_elements->length != 0) {
        tree->open_elements->length--;

        node = list[ tree->open_elements->length ];

        status |= (unsigned) lxb_html_tree_open_elements_pop_cb(node);

        switch (node->local_name) {
            case LXB_TAG_H1:
            case LXB_TAG_H2:
            case LXB_TAG_H3:
            case LXB_TAG_H4:
            case LXB_TAG_H5:
            case LXB_TAG_H6:
                if (node->ns == LXB_NS_HTML) {
                    return (status == LXB_STATUS_OK)
                            ? LXB_STATUS_OK
                            : LXB_STATUS_ERROR;
                }

                break;

            default:
                break;
        }
    }

    return (status == LXB_STATUS_OK) ? LXB_STATUS_OK : LXB_STATUS_ERROR;
}

lxb_status_t
lxb_html_tree_open_elements_pop_until_td_th(lxb_html_tree_t *tree)
{
    void **list;
    unsigned status;
    lxb_dom_node_t *node;

    list = tree->open_elements->list;
    status = LXB_STATUS_OK;

    while (tree->open_elements->length != 0) {
        tree->open_elements->length--;

        node = list[ tree->open_elements->length ];

        status |= (unsigned) lxb_html_tree_open_elements_pop_cb(node);

        switch (node->local_name) {
            case LXB_TAG_TD:
            case LXB_TAG_TH:
                if (node->ns == LXB_NS_HTML) {
                    return (status == LXB_STATUS_OK)
                                ? LXB_STATUS_OK
                                : LXB_STATUS_ERROR;
                }

                break;

            default:
                break;
        }
    }

    return (status == LXB_STATUS_OK) ? LXB_STATUS_OK : LXB_STATUS_ERROR;
}

lxb_status_t
lxb_html_tree_open_elements_pop_until_node(lxb_html_tree_t *tree,
                                           lxb_dom_node_t *node,
                                           bool exclude)
{
    void **list;
    unsigned status;
    lxb_dom_node_t *poped;

    list = tree->open_elements->list;
    status = LXB_STATUS_OK;

    while (tree->open_elements->length != 0) {
        poped = list[ --tree->open_elements->length ];

        if (poped == node) {
            if (exclude == false) {
                tree->open_elements->length++;
            }
            else {
                status |= (unsigned) lxb_html_tree_open_elements_pop_cb(poped);
            }

            break;
        }

        status |= (unsigned) lxb_html_tree_open_elements_pop_cb(poped);
    }

    return (status == LXB_STATUS_OK) ? LXB_STATUS_OK : LXB_STATUS_ERROR;
}

lxb_status_t
lxb_html_tree_open_elements_pop_all(lxb_html_tree_t *tree)
{
    void **list;
    unsigned status;
    lxb_dom_node_t *node;

    list = tree->open_elements->list;
    status = LXB_STATUS_OK;

    while (tree->open_elements->length != 0) {
        node = list[ --tree->open_elements->length ];
        status |= (unsigned) lxb_html_tree_open_elements_pop_cb(node);
    }

    return (status == LXB_STATUS_OK) ? LXB_STATUS_OK : LXB_STATUS_ERROR;
}

lxb_dom_node_t *
lxb_html_tree_open_elements_pop(lxb_html_tree_t *tree)
{
    lxb_dom_node_t *node;

    node = (lxb_dom_node_t *) lexbor_array_pop(tree->open_elements);

    if (node != NULL) {
        lxb_html_tree_open_elements_pop_cb(node);
    }

    return node;
}

bool
lxb_html_tree_open_elements_find_by_node(lxb_html_tree_t *tree,
                                         lxb_dom_node_t *node,
                                         size_t *return_pos)
{
    void **list = tree->open_elements->list;

    for (size_t i = 0; i < tree->open_elements->length; i++) {
        if (list[i] == node) {
            if (return_pos) {
                *return_pos = i;
            }

            return true;
        }
    }

    if (return_pos) {
        *return_pos = 0;
    }

    return false;
}

bool
lxb_html_tree_open_elements_find_by_node_reverse(lxb_html_tree_t *tree,
                                                 lxb_dom_node_t *node,
                                                 size_t *return_pos)
{
    void **list = tree->open_elements->list;
    size_t len = tree->open_elements->length;

    while (len != 0) {
        len--;

        if (list[len] == node) {
            if (return_pos) {
                *return_pos = len;
            }

            return true;
        }
    }

    if (return_pos) {
        *return_pos = 0;
    }

    return false;
}

lxb_dom_node_t *
lxb_html_tree_open_elements_find(lxb_html_tree_t *tree,
                                 lxb_tag_id_t tag_id, lxb_ns_id_t ns,
                                 size_t *return_index)
{
    void **list = tree->open_elements->list;
    lxb_dom_node_t *node;

    for (size_t i = 0; i < tree->open_elements->length; i++) {
        node = list[i];

        if (node->local_name == tag_id && node->ns == ns) {
            if (return_index) {
                *return_index = i;
            }

            return node;
        }
    }

    if (return_index) {
        *return_index = 0;
    }

    return NULL;
}

lxb_dom_node_t *
lxb_html_tree_open_elements_find_reverse(lxb_html_tree_t *tree,
                                         lxb_tag_id_t tag_id, lxb_ns_id_t ns,
                                         size_t *return_index)
{
    void **list = tree->open_elements->list;
    size_t len = tree->open_elements->length;

    lxb_dom_node_t *node;

    while (len != 0) {
        len--;
        node = list[len];

        if (node->local_name == tag_id && node->ns == ns) {
            if (return_index) {
                *return_index = len;
            }

            return node;
        }
    }

    if (return_index) {
        *return_index = 0;
    }

    return NULL;
}
