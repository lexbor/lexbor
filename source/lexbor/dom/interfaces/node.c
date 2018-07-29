/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/interfaces/node.h"
#include "lexbor/dom/interfaces/document.h"


lxb_dom_node_t *
lxb_dom_node_create(lxb_dom_document_t *document)
{
    lxb_dom_node_t *element;

    element = lexbor_mraw_calloc(document->mraw,
                                 sizeof(lxb_dom_node_t));
    if (element == NULL) {
        return NULL;
    }

    element->owner_document = document;
    element->type = LXB_DOM_NODE_TYPE_UNDEF;

    return element;
}

lxb_dom_node_t *
lxb_dom_node_destroy(lxb_dom_node_t *node)
{
    return lexbor_mraw_free(node->owner_document->mraw, node);
}

void
lxb_dom_node_insert_child(lxb_dom_node_t *to, lxb_dom_node_t *node)
{
    if (to->last_child != NULL) {
        to->last_child->next = node;
    }
    else {
        to->first_child = node;
    }

    node->parent = to;
    node->next = NULL;
    node->prev = to->last_child;

    to->last_child = node;
}

void
lxb_dom_node_insert_before(lxb_dom_node_t *to, lxb_dom_node_t *node)
{
    if (to->prev != NULL) {
        to->prev->next = node;
    }
    else {
        if (to->parent != NULL) {
            to->parent->first_child = node;
        }
    }

    node->parent = to->parent;
    node->next = to;
    node->prev = to->prev;

    to->prev = node;
}

void
lxb_dom_node_insert_after(lxb_dom_node_t *to, lxb_dom_node_t *node)
{
    if (to->next != NULL) {
        to->next->prev = node;
    }
    else {
        if (to->parent != NULL) {
            to->parent->last_child = node;
        }
    }

    node->parent = to->parent;
    node->next = to->next;
    node->prev = to;
    to->next = node;
}

void
lxb_dom_node_remove(lxb_dom_node_t *node)
{
    if (node->parent != NULL) {
        if (node->parent->first_child == node) {
            node->parent->first_child = node->next;
        }

        if (node->parent->last_child == node) {
            node->parent->last_child = node->prev;
        }
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
    }

    node->parent = NULL;
    node->next = NULL;
    node->prev = NULL;
}
