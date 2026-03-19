/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/element_steps.h"
#include "lexbor/dom/interfaces/document.h"

#include "lexbor/tag/const.h"
#include "lexbor/ns/const.h"

#include "lexbor/html/interfaces/option_element.h"
#include "lexbor/html/interfaces/select_element.h"
#include "lexbor/html/interfaces/selectedcontent_element.h"
#include "lexbor/html/element_steps_res.h"


/*
 * Element steps.
 */
lxb_status_t
lxb_html_element_steps_insertion(lxb_dom_node_t *inserted_node)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_cb_insertion_f inserted;

    if (inserted_node->ns != LXB_NS_HTML) {
        return LXB_STATUS_OK;
    }

    tag_id = inserted_node->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_inserted_unknown_steps(inserted_node);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    inserted = lxb_html_element_steps_res_default[inserted_node->local_name].inserted;

    if (inserted != NULL) {
        return inserted(inserted_node);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_steps_removing(lxb_dom_node_t *removed_node,
                                lxb_dom_node_t *old_parent)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_cb_removing_f removed;

    if (removed_node->ns != LXB_NS_HTML) {
        return LXB_STATUS_OK;
    }

    tag_id = removed_node->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_removed_unknown_steps(removed_node, old_parent);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    removed = lxb_html_element_steps_res_default[removed_node->local_name].removed;

    if (removed != NULL) {
        return removed(removed_node, old_parent);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_steps_moving(lxb_dom_node_t *moved_node,
                              lxb_dom_node_t *old_parent)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_cb_moving_f moved;

    if (moved_node->ns != LXB_NS_HTML) {
        return LXB_STATUS_OK;
    }

    tag_id = moved_node->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_moved_unknown_steps(moved_node, old_parent);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    moved = lxb_html_element_steps_res_default[moved_node->local_name].moved;

    if (moved != NULL) {
        return moved(moved_node, old_parent);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_steps_destroy(lxb_dom_node_t *node)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_cb_destroy_f destroy;

    if (node->ns != LXB_NS_HTML) {
        return LXB_STATUS_OK;
    }

    tag_id = node->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_destroy_unknown_steps(node);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    destroy = lxb_html_element_steps_res_default[node->local_name].destroy;

    if (destroy != NULL) {
        return destroy(node);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_steps_children_changed(lxb_dom_node_t *parent)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_cb_children_changed_f children_changed;

    if (parent->ns != LXB_NS_HTML) {
        return LXB_STATUS_OK;
    }

    tag_id = parent->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_children_changed_unknown_steps(parent);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    children_changed = lxb_html_element_steps_res_default[parent->local_name].children_changed;

    if (children_changed != NULL) {
        return children_changed(parent);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_steps_post_connection(lxb_dom_node_t *connected_node)
{
    lxb_tag_id_t tag_id;
    lxb_dom_node_cb_post_connection_f connected;

    if (connected_node->ns != LXB_NS_HTML) {
        return LXB_STATUS_OK;
    }

    tag_id = connected_node->local_name;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        if (tag_id == LXB_TAG__LAST_ENTRY) {
            return LXB_STATUS_OK;
        }

        return lxb_html_element_connected_unknown_steps(connected_node);
    }

    if (tag_id < LXB_TAG__BEGIN) {
        return LXB_STATUS_OK;
    }

    connected = lxb_html_element_steps_res_default[connected_node->local_name].connected;

    if (connected != NULL) {
        return connected(connected_node);
    }

    return LXB_STATUS_OK;
}
