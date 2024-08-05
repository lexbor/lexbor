/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/grammar/node.h"
#include "lexbor/grammar/tokenizer.h"
#include "lexbor/grammar/parser.h"

#include "lexbor/core/conv.h"
#include "lexbor/html/serialize.h"


lxb_grammar_node_t *
lxb_grammar_node_create(lxb_grammar_parser_t *parser, lxb_grammar_token_t *token,
                        lxb_grammar_node_type_t type)
{
    lxb_grammar_node_t *node;
    lxb_grammar_document_t *document = parser->document;

    node = lexbor_mraw_calloc(document->dom_document.mraw,
                              sizeof(lxb_grammar_node_t));
    if (node == NULL) {
        return NULL;
    }

    node->index = ++parser->index;
    node->type = type;
    node->token = token;
    node->document = parser->document;
    node->multiplier.start = 1;
    node->multiplier.stop = 1;

    if (token == NULL) {
        return node;
    }

    switch (type) {
        case LXB_GRAMMAR_NODE_DECLARATION:
        case LXB_GRAMMAR_NODE_ELEMENT:
            node->u.node = token->u.node;
            break;

        case LXB_GRAMMAR_NODE_NUMBER:
            node->u.num = token->u.num;
            break;

        case LXB_GRAMMAR_NODE_STRING:
        case LXB_GRAMMAR_NODE_WHITESPACE:
        case LXB_GRAMMAR_NODE_DELIM:
        case LXB_GRAMMAR_NODE_UNQUOTED:
            node->u.str = token->u.str;
            break;

        default:
            break;
    }

    return node;
}

void
lxb_grammar_node_clean(lxb_grammar_node_t *node)
{
    lxb_grammar_document_t *document = node->document;
    size_t index = node->index;

    memset(node, 0, sizeof(lxb_grammar_node_t));

    node->index = index;
    node->document = document;
}

lxb_grammar_node_t *
lxb_grammar_node_destroy(lxb_grammar_node_t *node)
{
    if (node == NULL) {
        return NULL;
    }

    return lexbor_mraw_free(node->document->dom_document.mraw, node);
}

void
lxb_grammar_node_insert_child(lxb_grammar_node_t *to, lxb_grammar_node_t *node)
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
lxb_grammar_node_insert_before(lxb_grammar_node_t *to, lxb_grammar_node_t *node)
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
lxb_grammar_node_insert_after(lxb_grammar_node_t *to, lxb_grammar_node_t *node)
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
lxb_grammar_node_remove(lxb_grammar_node_t *node)
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

lxb_inline lxb_status_t
lxb_grammar_node_serialize_combinator(lxb_grammar_node_t *group,
                                      lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;

    switch (group->combinator) {
        case LXB_GRAMMAR_COMBINATOR_AND:
            lxb_grammar_cb_m(" && ", 4);
            break;

        case LXB_GRAMMAR_COMBINATOR_OR:
            lxb_grammar_cb_m(" || ", 4);
            break;

        case LXB_GRAMMAR_COMBINATOR_ONE_OF:
            lxb_grammar_cb_m(" | ", 3);
            break;

        default:
            lxb_grammar_cb_m(" ", 1);
            break;
    }

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_grammar_node_serialize_combinator_wo_ws(const lxb_grammar_node_t *group,
                                     lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;

    switch (group->combinator) {
        case LXB_GRAMMAR_COMBINATOR_AND:
            lxb_grammar_cb_m("&&", 2);
            break;

        case LXB_GRAMMAR_COMBINATOR_OR:
            lxb_grammar_cb_m("||", 2);
            break;

        case LXB_GRAMMAR_COMBINATOR_ONE_OF:
            lxb_grammar_cb_m("|", 1);
            break;

        default:
            lxb_grammar_cb_m(" ", 1);
            break;
    }

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_grammar_node_serialize_multiplier(const lxb_grammar_node_t *node,
                                      lxb_grammar_serialize_cb_f func, void *ctx)
{
    size_t len;
    lxb_status_t status;
    const lxb_grammar_period_t *multiplier = &node->multiplier;
    lxb_char_t buf[128];

    if (multiplier->start == 1 && multiplier->stop == 1) {
        goto skip_ws;
    }

    if (multiplier->stop == -1) {
        if (multiplier->start == 0) {
            lxb_grammar_cb_m("*", 1);
        }
        else if (multiplier->start == 1) {
            if (node->is_comma) {
                lxb_grammar_cb_m("#", 1);
            }
            else {
                lxb_grammar_cb_m("+", 1);
            }
        }

        goto skip_ws;
    }

    if (multiplier->start == 1 && multiplier->stop == 0) {
        lxb_grammar_cb_m("!", 1);
        goto skip_ws;
    }
    else if (multiplier->start == 0 && multiplier->stop == 1) {
        lxb_grammar_cb_m("?", 1);
        goto skip_ws;
    }

    if (node->is_comma) {
        lxb_grammar_cb_m("#", 1);
    }

    lxb_grammar_cb_m("{", 1);

    len = lexbor_conv_float_to_data(multiplier->start, buf,
                                    (sizeof(buf) / sizeof(lxb_char_t)));
    lxb_grammar_cb_m(buf, len);

    if (multiplier->start == multiplier->stop) {
        lxb_grammar_cb_m("}", 1);

        goto skip_ws;
    }

    lxb_grammar_cb_m(",", 1);

    len = lexbor_conv_float_to_data(multiplier->stop, buf,
                                    (sizeof(buf) / sizeof(lxb_char_t)));

    lxb_grammar_cb_m(buf, len);
    lxb_grammar_cb_m("}", 1);

skip_ws:

    if (node->skip_ws) {
        lxb_grammar_cb_m("^WS", 3);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_grammar_node_serialize_deep(const lxb_grammar_node_t *root,
                                lxb_grammar_serialize_cb_f func, void *ctx)
{
    size_t length;
    lxb_status_t status;
    const lxb_char_t *name;
    const lxb_grammar_node_t *node;
    lxb_dom_attr_t *attr;

    node = root;

    if (node->type == LXB_GRAMMAR_NODE_ROOT) {
        node = node->first_child;
    }

    while (node != NULL) {

        switch (node->type) {
            case LXB_GRAMMAR_NODE_GROUP:
                if (node->prev == NULL) {
                    lxb_grammar_cb_m("[", 1);
                }
                else {
                    status = lxb_grammar_node_serialize_combinator(node->parent,
                                                                   func, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }

                    lxb_grammar_cb_m("[", 1);
                }

                if (node->first_child != NULL) {
                    node = node->first_child;
                    continue;
                }

                lxb_grammar_cb_m("]", 1);

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                break;

            case LXB_GRAMMAR_NODE_DECLARATION:
                lxb_grammar_cb_m("<", 1);

                status = lxb_grammar_node_serialize(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                lxb_grammar_cb_m("> = ", 4);

                node = node->first_child;

                continue;

            default:
                if (node->prev) {
                    status = lxb_grammar_node_serialize_combinator(node->parent,
                                                                   func, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }

                if (node->type == LXB_GRAMMAR_NODE_ELEMENT) {
                    lxb_grammar_cb_m("<", 1);
                }

                status = lxb_grammar_node_serialize(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                if (node->type == LXB_GRAMMAR_NODE_ELEMENT) {
                    attr = lxb_dom_interface_element(node->u.node)->first_attr;

                    while (attr != NULL) {
                        lxb_grammar_cb_m(" ", 1);

                        name = lxb_dom_attr_local_name(attr, &length);
                        lxb_grammar_cb_m(name, length);

                        name = lxb_dom_attr_value(attr, &length);
                        if (name != NULL) {
                            lxb_grammar_cb_m("=\"", 2);
                            lxb_grammar_cb_m(name, length);
                            lxb_grammar_cb_m("\"", 1);
                        }

                        attr = attr->next;
                    }

                    lxb_grammar_cb_m(">", 1);
                }

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                break;
        }

        while (node->next == NULL) {
            node = node->parent;

            if (node == NULL) {
                return LXB_STATUS_ERROR;
            }

            if (node->type == LXB_GRAMMAR_NODE_DECLARATION) {
                if (node->next != NULL) {
                    lxb_grammar_cb_m("\n", 1);
                }
            }
            else if (node->type != LXB_GRAMMAR_NODE_ROOT) {
                lxb_grammar_cb_m("]", 1);

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }
            }

            if (node == root) {
                return LXB_STATUS_OK;
            }
        }

        node = node->next;
    }

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_grammar_node_escape_string(const lxb_char_t *data, size_t length,
                               lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;
    const lxb_char_t *begin = data;
    const lxb_char_t *end = data + length;

    while (data < end) {
        switch (*data) {
            case '\n':
                lxb_grammar_cb_m(begin, (data - begin));
                lxb_grammar_cb_m("\\n", 2);

                data++;
                begin = data;

                continue;

            case '"':
            case '\\':
                lxb_grammar_cb_m(begin, (data - begin));
                lxb_grammar_cb_m("\\", 1);

                begin = data;
                break;
        }

        data++;
    }

    if (begin < data) {
        lxb_grammar_cb_m(begin, (end - begin));
    }

    return LXB_STATUS_OK;
}


lxb_status_t
lxb_grammar_node_serialize(const lxb_grammar_node_t *node,
                           lxb_grammar_serialize_cb_f func, void *ctx)
{
    size_t len;
    lxb_char_t buf[128];
    lxb_status_t status;
    const lxb_tag_data_t *tag_data;

    switch (node->type) {
        case LXB_GRAMMAR_NODE_WHITESPACE:
        case LXB_GRAMMAR_NODE_DELIM:
        case LXB_GRAMMAR_NODE_UNQUOTED:
            return func(node->u.str.data, node->u.str.length, ctx);

        case LXB_GRAMMAR_NODE_DECLARATION:
        case LXB_GRAMMAR_NODE_ELEMENT: {
            tag_data = lxb_tag_data_by_id(node->u.node->local_name);
            if (tag_data == NULL) {
                return LXB_STATUS_ERROR;
            }

            return func(lexbor_hash_entry_str(&tag_data->entry),
                        tag_data->entry.length, ctx);
        }

        case LXB_GRAMMAR_NODE_STRING:
            lxb_grammar_cb_m("\"", 1);

            status = lxb_grammar_node_escape_string(node->u.str.data,
                                                    node->u.str.length,
                                                    func, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            return func((lxb_char_t *) "\"", 1, ctx);

        case LXB_GRAMMAR_NODE_NUMBER:
            len = lexbor_conv_float_to_data(node->u.num, buf,
                                            (sizeof(buf) / sizeof(lxb_char_t)));
            return func(buf, len, ctx);

        case LXB_GRAMMAR_NODE_ROOT:
            return func((lxb_char_t *) "#ROOT", 5, ctx);

        case LXB_GRAMMAR_NODE_LAST_ENTRY:
            return func((lxb_char_t *) "#END", 4, ctx);

        default:
            return func((lxb_char_t *) "UNDEFINED", 9, ctx);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_grammar_node_serialize_ast(const lxb_grammar_node_t *root,
                               lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;
    const lxb_grammar_node_t *node;

    size_t indent = 0;

    node = root;

    if (node->type == LXB_GRAMMAR_NODE_ROOT) {
        node = node->first_child;
    }

    while (node != NULL) {

        switch (node->type) {
            case LXB_GRAMMAR_NODE_GROUP:
                lxb_grammar_indent_m(indent);
                lxb_grammar_cb_m("<#GROUP>", 8);

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                if (node->combinator) {
                    lxb_grammar_cb_m(", ", 2);

                    status = lxb_grammar_node_serialize_combinator_wo_ws(node,
                                                                     func, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }

                lxb_grammar_cb_m("\n", 1);

                indent++;

                if (node->first_child != NULL) {
                    node = node->first_child;
                    continue;
                }

                break;

            case LXB_GRAMMAR_NODE_DECLARATION:
                lxb_grammar_indent_m(indent);
                lxb_grammar_cb_m("<", 1);

                status = lxb_grammar_node_serialize(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                lxb_grammar_cb_m(">", 1);

                if (node->combinator) {
                    lxb_grammar_cb_m(", ", 2);

                    status = lxb_grammar_node_serialize_combinator_wo_ws(node,
                                                                     func, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }

                lxb_grammar_cb_m("\n", 1);

                node = node->first_child;

                indent++;
                continue;

            case LXB_GRAMMAR_NODE_ELEMENT:
                lxb_grammar_indent_m(indent);
                lxb_grammar_cb_m("<", 1);

                status = lxb_grammar_node_serialize(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                lxb_grammar_cb_m(">", 1);

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                lxb_grammar_cb_m("\n", 1);

                break;

            default:
                lxb_grammar_indent_m(indent);

                status = lxb_grammar_node_serialize(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                lxb_grammar_cb_m("\n", 1);

                break;
        }

        while (node->next == NULL) {
            node = node->parent;

            indent--;

            if (node == NULL) {
                return LXB_STATUS_ERROR;
            }

            if (node == root) {
                return LXB_STATUS_OK;
            }
        }

        node = node->next;
    }

    return LXB_STATUS_OK;
}
