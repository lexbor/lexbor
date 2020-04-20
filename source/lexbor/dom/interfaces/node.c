/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/dom/interfaces/node.h"
#include "lexbor/dom/interfaces/attr.h"
#include "lexbor/dom/interfaces/document.h"
#include "lexbor/dom/interfaces/document_type.h"
#include "lexbor/dom/interfaces/element.h"
#include "lexbor/dom/interfaces/processing_instruction.h"


static lexbor_action_t
lxb_dom_node_text_content_size(lxb_dom_node_t *node, void *ctx);

static lexbor_action_t
lxb_dom_node_text_content_concatenate(lxb_dom_node_t *node, void *ctx);


lxb_dom_node_t *
lxb_dom_node_interface_create(lxb_dom_document_t *document)
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
lxb_dom_node_interface_destroy(lxb_dom_node_t *node)
{
    return lexbor_mraw_free(node->owner_document->mraw, node);
}

lxb_dom_node_t *
lxb_dom_node_destroy(lxb_dom_node_t *node)
{
    lxb_dom_node_remove(node);

    return lxb_dom_document_destroy_interface(node);
}

lxb_dom_node_t *
lxb_dom_node_destroy_deep(lxb_dom_node_t *root)
{
    lxb_dom_node_t *tmp;
    lxb_dom_node_t *node = root;

    while (node != NULL) {
        if (node->first_child != NULL) {
            node = node->first_child;
        }
        else {
            while(node != root && node->next == NULL) {
                tmp = node->parent;

                lxb_dom_node_destroy(node);

                node = tmp;
            }

            if (node == root) {
                lxb_dom_node_destroy(node);

                break;
            }

            tmp = node->next;

            lxb_dom_node_destroy(node);

            node = tmp;
        }
    }

    return NULL;
}

const lxb_char_t *
lxb_dom_node_name(lxb_dom_node_t *node, size_t *len)
{
    switch (node->type) {
        case LXB_DOM_NODE_TYPE_ELEMENT:
            return lxb_dom_element_tag_name(lxb_dom_interface_element(node),
                                            len);

        case LXB_DOM_NODE_TYPE_ATTRIBUTE:
            return lxb_dom_attr_qualified_name(lxb_dom_interface_attr(node),
                                               len);

        case LXB_DOM_NODE_TYPE_TEXT:
            if (len != NULL) {
                *len = sizeof("#text") - 1;
            }

            return (const lxb_char_t *) "#text";

        case LXB_DOM_NODE_TYPE_CDATA_SECTION:
            if (len != NULL) {
                *len = sizeof("#cdata-section") - 1;
            }

            return (const lxb_char_t *) "#cdata-section";

        case LXB_DOM_NODE_TYPE_PROCESSING_INSTRUCTION:
            return lxb_dom_processing_instruction_target(lxb_dom_interface_processing_instruction(node),
                                                         len);

        case LXB_DOM_NODE_TYPE_COMMENT:
            if (len != NULL) {
                *len = sizeof("#comment") - 1;
            }

            return (const lxb_char_t *) "#comment";

        case LXB_DOM_NODE_TYPE_DOCUMENT:
            if (len != NULL) {
                *len = sizeof("#document") - 1;
            }

            return (const lxb_char_t *) "#document";

        case LXB_DOM_NODE_TYPE_DOCUMENT_TYPE:
            return lxb_dom_document_type_name(lxb_dom_interface_document_type(node),
                                              len);

        case LXB_DOM_NODE_TYPE_DOCUMENT_FRAGMENT:
            if (len != NULL) {
                *len = sizeof("#document-fragment") - 1;
            }

            return (const lxb_char_t *) "#document-fragment";

        default:
            break;
    }

    if (len != NULL) {
        *len = 0;
    }

    return NULL;
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

lxb_status_t
lxb_dom_node_replace_all(lxb_dom_node_t *parent, lxb_dom_node_t *node)
{
    while (parent->first_child != NULL) {
        lxb_dom_node_destroy_deep(parent->first_child);
    }

    lxb_dom_node_insert_child(parent, node);

    return LXB_STATUS_OK;
}

void
lxb_dom_node_simple_walk(lxb_dom_node_t *root,
                         lxb_dom_node_simple_walker_f walker_cb, void *ctx)
{
    lexbor_action_t action;
    lxb_dom_node_t *node = root->first_child;

    while (node != NULL) {
        action = walker_cb(node, ctx);
        if (action == LEXBOR_ACTION_STOP) {
            return;
        }

        if (node->first_child != NULL && action != LEXBOR_ACTION_NEXT) {
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
}

lxb_char_t *
lxb_dom_node_text_content(lxb_dom_node_t *node, size_t *len)
{
    lxb_char_t *text;
    size_t length = 0;

    switch (node->type) {
        case LXB_DOM_NODE_TYPE_DOCUMENT_FRAGMENT:
        case LXB_DOM_NODE_TYPE_ELEMENT:
            lxb_dom_node_simple_walk(node, lxb_dom_node_text_content_size,
                                     &length);

            text = lxb_dom_document_create_text(node->owner_document,
                                                (length + 1));
            if (text == NULL) {
                goto failed;
            }

            lxb_dom_node_simple_walk(node, lxb_dom_node_text_content_concatenate,
                                     &text);

            text -= length;

            break;

        case LXB_DOM_NODE_TYPE_ATTRIBUTE: {
            const lxb_char_t *attr_text;

            attr_text = lxb_dom_attr_value(lxb_dom_interface_attr(node), &length);
            if (attr_text == NULL) {
                goto failed;
            }

            text = lxb_dom_document_create_text(node->owner_document,
                                                (length + 1));
            if (text == NULL) {
                goto failed;
            }

            /* +1 == with null '\0' */
            memcpy(text, attr_text, sizeof(lxb_char_t) * (length + 1));

            break;
        }

        case LXB_DOM_NODE_TYPE_TEXT:
        case LXB_DOM_NODE_TYPE_PROCESSING_INSTRUCTION:
        case LXB_DOM_NODE_TYPE_COMMENT: {
            lxb_dom_character_data_t *ch_data;

            ch_data = lxb_dom_interface_character_data(node);
            length = ch_data->data.length;

            text = lxb_dom_document_create_text(node->owner_document,
                                                (length + 1));
            if (text == NULL) {
                goto failed;
            }

            /* +1 == with null '\0' */
            memcpy(text, ch_data->data.data, sizeof(lxb_char_t) * (length + 1));

            break;
        }

        default:
            goto failed;
    }

    if (len != NULL) {
        *len = length;
    }

    text[length] = 0x00;

    return text;

failed:

    if (len != NULL) {
        *len = 0;
    }

    return NULL;
}

static lexbor_action_t
lxb_dom_node_text_content_size(lxb_dom_node_t *node, void *ctx)
{
    if (node->type == LXB_DOM_NODE_TYPE_TEXT) {
        *((size_t *) ctx) += lxb_dom_interface_text(node)->char_data.data.length;
    }

    return LEXBOR_ACTION_OK;
}

static lexbor_action_t
lxb_dom_node_text_content_concatenate(lxb_dom_node_t *node, void *ctx)
{
    if (node->type != LXB_DOM_NODE_TYPE_TEXT) {
        return LEXBOR_ACTION_OK;
    }

    lxb_char_t **text = (lxb_char_t **) ctx;
    lxb_dom_character_data_t *ch_data = &lxb_dom_interface_text(node)->char_data;

    memcpy(*text, ch_data->data.data, sizeof(lxb_char_t) * ch_data->data.length);

    *text = *text + ch_data->data.length;

    return LEXBOR_ACTION_OK;
}

lxb_status_t
lxb_dom_node_text_content_set(lxb_dom_node_t *node,
                              const lxb_char_t *content, size_t len)
{
    lxb_status_t status;

    switch (node->type) {
        case LXB_DOM_NODE_TYPE_DOCUMENT_FRAGMENT:
        case LXB_DOM_NODE_TYPE_ELEMENT: {
            lxb_dom_text_t *text;

            text = lxb_dom_document_create_text_node(node->owner_document,
                                                     content, len);
            if (text == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            status = lxb_dom_node_replace_all(node, lxb_dom_interface_node(text));
            if (status != LXB_STATUS_OK) {
                lxb_dom_document_destroy_interface(text);

                return status;
            }

            break;
        }

        case LXB_DOM_NODE_TYPE_ATTRIBUTE:
            return lxb_dom_attr_set_existing_value(lxb_dom_interface_attr(node),
                                                   content, len);

        case LXB_DOM_NODE_TYPE_TEXT:
        case LXB_DOM_NODE_TYPE_PROCESSING_INSTRUCTION:
        case LXB_DOM_NODE_TYPE_COMMENT:
            return lxb_dom_character_data_replace(lxb_dom_interface_character_data(node),
                                                  content, len, 0, 0);

        default:
            return LXB_STATUS_OK;
    }

    return LXB_STATUS_OK;
}

lxb_tag_id_t
lxb_dom_node_tag_id_noi(lxb_dom_node_t *node)
{
    return lxb_dom_node_tag_id(node);
}

lxb_dom_node_t *
lxb_dom_node_next_noi(lxb_dom_node_t *node)
{
    return lxb_dom_node_next(node);
}

lxb_dom_node_t *
lxb_dom_node_prev_noi(lxb_dom_node_t *node)
{
    return lxb_dom_node_prev(node);
}

lxb_dom_node_t *
lxb_dom_node_parent_noi(lxb_dom_node_t *node)
{
    return lxb_dom_node_parent(node);
}

lxb_dom_node_t *
lxb_dom_node_first_child_noi(lxb_dom_node_t *node)
{
    return lxb_dom_node_first_child(node);
}

lxb_dom_node_t *
lxb_dom_node_last_child_noi(lxb_dom_node_t *node)
{
    return lxb_dom_node_last_child(node);
}
