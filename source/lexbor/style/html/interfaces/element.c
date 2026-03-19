/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/html/interfaces/element.h"
#include "lexbor/style/style.h"


static lexbor_action_t
lxb_style_html_element_ditry_cb(lxb_dom_node_t *node, void *ctx);

static lxb_status_t
lxb_style_html_element_remove_all_styles_cb(lexbor_avl_t *avl,
                                            lexbor_avl_node_t **root,
                                            lexbor_avl_node_t *node, void *ctx);

static lxb_status_t
lxb_style_html_element_steps_remove_my_cb(lexbor_avl_t *avl,
                                          lexbor_avl_node_t **root,
                                          lexbor_avl_node_t *node, void *ctx);


/*
 * Element steps.
 */
lxb_status_t
lxb_style_html_element_inserted_steps(lxb_dom_node_t *inserted_node)
{
    lxb_status_t status;
    lxb_dom_element_t *el = lxb_dom_interface_element(inserted_node);

    if (el->condition & LXB_DOM_ELEMENT_CONDITION_DIRTY_STYLE) {
        status = lxb_dom_element_style_remove_non_inline(el);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        el->condition &= ~LXB_DOM_ELEMENT_CONDITION_DIRTY_STYLE;
    }

    return lxb_dom_document_element_styles_attach(el);
}

/*
 * If a node has children, the extraction function will not be called for them;
 * it will only be called for the node being extracted from the tree.
 * Therefore, we need to mark the child elements as "dirty" so that the applied
 * styles will be removed later (not immediately). This will be done lazily.
 */
lxb_status_t
lxb_style_html_element_removed_steps(lxb_dom_node_t *removed_node,
                                     lxb_dom_node_t *old_parent)
{
    lxb_dom_element_t *el = lxb_dom_interface_element(removed_node);

    lxb_dom_node_simple_walk(removed_node,
                             lxb_style_html_element_ditry_cb, NULL);

    if (el->style == NULL) {
        return LXB_STATUS_OK;
    }

    el->condition |= LXB_DOM_ELEMENT_CONDITION_DIRTY_STYLE;

    return LXB_STATUS_OK;
}

static lexbor_action_t
lxb_style_html_element_ditry_cb(lxb_dom_node_t *node, void *ctx)
{
    lxb_dom_element_t *el;

    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        el = lxb_dom_interface_element(node);
        el->condition |= LXB_DOM_ELEMENT_CONDITION_DIRTY_STYLE;
    }

    return LEXBOR_ACTION_OK;
}

lxb_status_t
lxb_style_html_element_moved_steps(lxb_dom_node_t *moved_node,
                                   lxb_dom_node_t *old_parent)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_html_element_destroy_steps(lxb_dom_node_t *node)
{
    lxb_status_t status;
    lxb_dom_element_t *el;
    lxb_dom_document_t *doc;
    lxb_style_html_steps_ctx_t context;

    el = lxb_dom_interface_element(node);

    if (el->style == NULL) {
        if (el->list != NULL) {
            goto destroy;
        }

        return LXB_STATUS_OK;
    }

    doc = node->owner_document;

    context.element = el;
    context.all = true;

    status = lexbor_avl_foreach(doc->css->styles, &el->style,
                                lxb_style_html_element_remove_all_styles_cb,
                                &context);

    if (status != LXB_STATUS_OK) {
        return status;
    }

destroy:

    ((lxb_css_rule_declaration_list_t *) (el->list))->first = NULL;
    ((lxb_css_rule_declaration_list_t *) (el->list))->last = NULL;

    el->list = lxb_css_rule_declaration_list_destroy(el->list, true);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_style_html_element_remove_all_styles_cb(lexbor_avl_t *avl,
                                            lexbor_avl_node_t **root,
                                            lexbor_avl_node_t *node, void *ctx)
{
    lxb_style_html_steps_ctx_t *context = ctx;

    lxb_dom_element_style_remove_all(context->element,
                                     (lxb_style_node_t *) node);
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_html_element_children_changed_steps(lxb_dom_node_t *parent)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_html_element_connected_steps(lxb_dom_node_t *connected_node)
{
    return LXB_STATUS_OK;
}

/*
 * Unknown Element steps.
 */
lxb_status_t
lxb_style_html_element_inserted_unknown_steps(lxb_dom_node_t *inserted_node)
{
    return lxb_style_html_element_inserted_steps(inserted_node);
}

lxb_status_t
lxb_style_html_element_removed_unknown_steps(lxb_dom_node_t *removed_node,
                                             lxb_dom_node_t *old_parent)
{
    return lxb_style_html_element_removed_steps(removed_node, old_parent);
}

lxb_status_t
lxb_style_html_element_moved_unknown_steps(lxb_dom_node_t *moved_node,
                                           lxb_dom_node_t *old_parent)
{
    return lxb_style_html_element_moved_steps(moved_node, old_parent);
}

lxb_status_t
lxb_style_html_element_destroy_unknown_steps(lxb_dom_node_t *node)
{
    return lxb_style_html_element_destroy_steps(node);
}

lxb_status_t
lxb_style_html_element_children_changed_unknown_steps(lxb_dom_node_t *parent)
{
    return lxb_style_html_element_children_changed_steps(parent);
}

lxb_status_t
lxb_style_html_element_connected_unknown_steps(lxb_dom_node_t *connected_node)
{
    return lxb_style_html_element_connected_steps(connected_node);
}

lxb_status_t
lxb_style_html_element_attr_change(lxb_dom_element_t *element,
                                   lxb_dom_attr_id_t local_name,
                                   const lxb_char_t *old_value, size_t old_len,
                                   const lxb_char_t *value, size_t value_len,
                                   lxb_ns_id_t ns)
{
    return lxb_style_html_element_attr_append(element, local_name, old_value,
                                              old_len, value, value_len, ns);
}

lxb_status_t
lxb_style_html_element_attr_append(lxb_dom_element_t *element,
                                   lxb_dom_attr_id_t local_name,
                                   const lxb_char_t *old_value, size_t old_len,
                                   const lxb_char_t *value, size_t value_len,
                                   lxb_ns_id_t ns)
{
    lxb_status_t status;

    if (local_name != LXB_DOM_ATTR_STYLE) {
        return LXB_STATUS_OK;
    }

    if (element->list != NULL) {
        status = lxb_style_html_element_attr_remove(element, local_name,
                                                    old_value, old_len,
                                                    value, value_len, ns);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (value == NULL || value_len == 0) {
        return LXB_STATUS_OK;
    }

    return lxb_dom_element_style_parse(element, value, value_len);
}

lxb_status_t
lxb_style_html_element_attr_remove(lxb_dom_element_t *element,
                                   lxb_dom_attr_id_t local_name,
                                   const lxb_char_t *old_value, size_t old_len,
                                   const lxb_char_t *value, size_t value_len,
                                   lxb_ns_id_t ns)
{
    lxb_status_t status;
    lxb_dom_document_t *doc;
    lxb_style_html_steps_ctx_t context;

    if (local_name != LXB_DOM_ATTR_STYLE) {
        return LXB_STATUS_OK;
    }

    if (element->list == NULL) {
        return LXB_STATUS_OK;
    }

    doc = lxb_dom_element_document(element);
    context.element = element;

    status = lexbor_avl_foreach(doc->css->styles, &element->style,
                                lxb_style_html_element_steps_remove_my_cb,
                                &context);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    ((lxb_css_rule_declaration_list_t *) (element->list))->first = NULL;
    ((lxb_css_rule_declaration_list_t *) (element->list))->last = NULL;

    element->list = lxb_css_rule_declaration_list_destroy(element->list, true);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_html_element_attr_replace(lxb_dom_element_t *element,
                                    lxb_dom_attr_id_t local_name,
                                    const lxb_char_t *old_value, size_t old_len,
                                    const lxb_char_t *value, size_t value_len,
                                    lxb_ns_id_t ns)
{
    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_style_html_element_steps_remove_my_cb(lexbor_avl_t *avl,
                                          lexbor_avl_node_t **root,
                                          lexbor_avl_node_t *node, void *ctx)
{
    lxb_style_html_steps_ctx_t *context = ctx;

    lxb_dom_element_style_remove_all_not(context->element,
                                         (lxb_style_node_t *) node, true);
    return LXB_STATUS_OK;
}
