/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/event.h"
#include "lexbor/style/dom/interfaces/element.h"
#include "lexbor/style/dom/interfaces/document.h"
#include "lexbor/style/html/interfaces/style_element.h"


typedef struct {
    lxb_dom_document_t *doc;
    bool               all;
}
lxb_style_event_ctx_t;


static lxb_status_t
lxb_style_event_remove_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                          lexbor_avl_node_t *node, void *ctx);

static lxb_status_t
lxb_style_event_remove_my_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                             lexbor_avl_node_t *node, void *ctx);


lxb_status_t
lxb_style_event_insert(lxb_dom_node_t *node)
{
    if (node->type == LXB_DOM_NODE_TYPE_ATTRIBUTE) {
        return lxb_style_event_insert_attribute(node);
    }
    else if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        return lxb_style_event_insert_element(node);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_event_insert_element(lxb_dom_node_t *node)
{
    lxb_status_t status;
    lxb_html_style_element_t *style;

    if (lxb_html_node_is(node, LXB_TAG_STYLE)) {
        style = lxb_html_interface_style(node);

        status = lxb_html_style_element_parse(style);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        status = lxb_dom_document_stylesheet_attach(node->owner_document,
                                                    style->stylesheet);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_dom_document_element_styles_attach(lxb_dom_interface_element(node));
}

lxb_status_t
lxb_style_event_insert_attribute(lxb_dom_node_t *node)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_element_t *el;

    if (node->local_name != LXB_DOM_ATTR_STYLE) {
        return LXB_STATUS_OK;
    }

    attr = lxb_dom_interface_attr(node);
    el = lxb_dom_interface_element(attr->owner);

    if (el != NULL && el->list != NULL) {
        status = lxb_style_event_remove_attribute(node);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (attr->value == NULL || attr->value->data == NULL) {
        return LXB_STATUS_OK;
    }

    return lxb_dom_element_style_parse(el, attr->value->data,
                                       attr->value->length);
}

lxb_status_t
lxb_style_event_remove(lxb_dom_node_t *node)
{
    if (node->type == LXB_DOM_NODE_TYPE_ATTRIBUTE) {
        return lxb_style_event_remove_attribute(node);
    }
    else if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        return lxb_style_event_remove_element(node);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_event_remove_element(lxb_dom_node_t *node)
{
    lxb_status_t status;
    lxb_dom_element_t *el;
    lxb_dom_document_t *doc;
    lxb_style_event_ctx_t context;

    if (lxb_html_node_is(node, LXB_TAG_STYLE)) {
        status = lxb_html_style_element_remove(lxb_html_interface_style(node));
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    el = lxb_dom_interface_element(node);

    if (el->style == NULL) {
        return LXB_STATUS_OK;
    }

    doc = node->owner_document;

    context.doc = doc;
    context.all = false;

    return lexbor_avl_foreach(doc->css->styles, &el->style,
                              lxb_style_event_remove_cb, &context);
}

static lxb_status_t
lxb_style_event_remove_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                          lexbor_avl_node_t *node, void *ctx)
{
    lxb_style_event_ctx_t *context = ctx;
    lxb_style_node_t *style = (lxb_style_node_t *) node;

    if (context->all) {
        lxb_dom_element_style_remove_all(context->doc, root, style);
    }
    else {
        lxb_dom_element_style_remove_all_not(context->doc, root, style, false);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_event_remove_attribute(lxb_dom_node_t *node)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_dom_element_t *el;
    lxb_dom_document_t *doc;
    lxb_style_event_ctx_t context;

    if (node->local_name != LXB_DOM_ATTR_STYLE) {
        return LXB_STATUS_OK;
    }

    attr = lxb_dom_interface_attr(node);
    el = lxb_dom_interface_element(attr->owner);

    if (el == NULL || el->list == NULL) {
        return LXB_STATUS_OK;
    }

    doc = node->owner_document;

    context.doc = doc;

    status = lexbor_avl_foreach(doc->css->styles, &el->style,
                                lxb_style_event_remove_my_cb, &context);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    ((lxb_css_rule_declaration_list_t *) (el->list))->first = NULL;
    ((lxb_css_rule_declaration_list_t *) (el->list))->last = NULL;

    el->list = lxb_css_rule_declaration_list_destroy(el->list, true);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_style_event_remove_my_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                             lexbor_avl_node_t *node, void *ctx)
{
    lxb_style_event_ctx_t *context = ctx;
    lxb_style_node_t *style = (lxb_style_node_t *) node;

    lxb_dom_element_style_remove_all_not(context->doc, root, style, true);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_event_destroy(lxb_dom_node_t *node)
{
    if (node->type == LXB_DOM_NODE_TYPE_ATTRIBUTE) {
        return lxb_style_event_destroy_attribute(node);
    }
    else if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        return lxb_style_event_destroy_element(node);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_event_destroy_element(lxb_dom_node_t *node)
{
    lxb_status_t status;
    lxb_dom_element_t *el;
    lxb_dom_document_t *doc;
    lxb_style_event_ctx_t context;

    el = lxb_dom_interface_element(node);

    if (el->style == NULL) {
        if (el->list != NULL) {
            goto destroy;
        }

        return LXB_STATUS_OK;
    }

    doc = node->owner_document;

    context.doc = doc;
    context.all = true;

    status = lexbor_avl_foreach(doc->css->styles, &el->style,
                                lxb_style_event_remove_cb, &context);

    if (status != LXB_STATUS_OK) {
        return status;
    }

destroy:

    ((lxb_css_rule_declaration_list_t *) (el->list))->first = NULL;
    ((lxb_css_rule_declaration_list_t *) (el->list))->last = NULL;

    el->list = lxb_css_rule_declaration_list_destroy(el->list, true);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_style_event_destroy_attribute(lxb_dom_node_t *node)
{
    return lxb_style_event_remove_attribute(node);
}

lxb_status_t
lxb_style_event_set_value(lxb_dom_node_t *node,
                          const lxb_char_t *value, size_t length)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr = lxb_dom_interface_attr(node);

    if (node->type != LXB_DOM_NODE_TYPE_ATTRIBUTE
        || node->local_name != LXB_DOM_ATTR_STYLE)
    {
        return LXB_STATUS_OK;
    }

    if (attr->owner == NULL) {
        return LXB_STATUS_OK;
    }

    status = lxb_style_event_remove_attribute(node);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_dom_element_style_parse(lxb_dom_interface_element(node),
                                       value, length);
}
