/*
* Copyright (C) 2020 Alexander Borisov
*
* Author: Alexander Borisov <borisov@lexbor.com>
*/

#include "lexbor/grammar/json.h"
#include "lexbor/grammar/node.h"

#include "lexbor/core/conv.h"

#include "lexbor/html/serialize.h"


#define lxb_grammar_json_escape_cb_m(text, length)                             \
    do {                                                                       \
        status = lxb_grammar_json_escape((const lxb_char_t *) (text), length,  \
                                         func, ctx);                           \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)


lxb_status_t
lxb_grammar_json_init(lxb_grammar_json_t *json)
{
    if (json == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    json->scope_root = NULL;

    return lexbor_avl_init(&json->avl, 32, sizeof(lexbor_avl_node_t));
}

lxb_grammar_json_t *
lxb_grammar_json_destroy(lxb_grammar_json_t *json, bool self_destroy)
{
    if (json == NULL) {
        return NULL;
    }

    lexbor_avl_destroy(&json->avl, false);

    if (self_destroy) {
        return lexbor_free(json);
    }

    return json;
}

lxb_inline lxb_status_t
lxb_grammar_json_escape(const lxb_char_t *data, size_t length,
                        lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;
    const lxb_char_t *begin = data;
    const lxb_char_t *end = data + length;

    while (data < end) {
        switch (*data) {
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

lxb_inline lxb_status_t
lxb_grammar_json_ast_combinator(const lxb_grammar_node_t *group,
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

lxb_status_t
lxb_grammar_json_ast(const lxb_grammar_node_t *root, size_t indent,
                     lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;
    const lxb_grammar_node_t *node;

    node = root;

    if (node->type == LXB_GRAMMAR_NODE_ROOT) {
        node = node->first_child;
    }

    lxb_grammar_cb_m("[\n", 2);

    indent++;

    while (node != NULL) {
        lxb_grammar_indent_m(indent);
        lxb_grammar_cb_m("{\n", 2);

        status = lxb_grammar_json_ast_node_attributes(node, indent + 1,
                                                      func, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        if (node->type == LXB_GRAMMAR_NODE_DECLARATION
            || node->type == LXB_GRAMMAR_NODE_GROUP)
        {
            lxb_grammar_cb_m(",\n", 2);
            lxb_grammar_indent_m(indent + 1);

            if (node->first_child != NULL) {
                lxb_grammar_cb_m("\"nodes\": [\n", sizeof("\"nodes\": [\n") - 1);

                indent += 2;

                node = node->first_child;
                continue;
            }

            lxb_grammar_cb_m("\"nodes\": []\n", sizeof("\"nodes\": []\n") - 1);
        }
        else {
            lxb_grammar_cb_m("\n", 1);
            lxb_grammar_indent_m(indent);

            if (node->next == NULL) {
                lxb_grammar_cb_m("}\n", 2);
            }
            else {
                lxb_grammar_cb_m("},\n", 3);
            }
        }

        while (node->next == NULL) {
            node = node->parent;

            if (node == NULL) {
                return LXB_STATUS_ERROR;
            }

            if (node == root) {
                lxb_grammar_indent_m(indent - 1);
                lxb_grammar_cb_m("]", 1);

                return LXB_STATUS_OK;
            }

            lxb_grammar_indent_m(indent - 1);
            lxb_grammar_cb_m("]\n", 2);

            lxb_grammar_indent_m(indent - 2);

            if (node->next == NULL) {
                lxb_grammar_cb_m("}\n", 2);
            }
            else {
                lxb_grammar_cb_m("},\n", 3);
            }

            indent -= 2;
        }

        node = node->next;
    }

    lxb_grammar_cb_m("]", 1);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_grammar_json_ast_node_attributes(const lxb_grammar_node_t *node, size_t indent,
                                     lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;

    size_t len;
    lxb_char_t buf[32];

    /* id */
    lxb_grammar_indent_m(indent);

    if (node->type == LXB_GRAMMAR_NODE_DECLARATION) {
        lxb_grammar_cb_m("\"id\": ", sizeof("\"id\": ") - 1);

        status = lxb_grammar_json_ast_node_value(node, func, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        lxb_grammar_cb_m(",\n", 2);
    }
    else {
        lxb_grammar_cb_m("\"id\": \"", sizeof("\"id\": \"") - 1);

        len = lexbor_conv_float_to_data((double) node->index, buf, sizeof(buf));

        lxb_grammar_cb_m(buf, len);
        lxb_grammar_cb_m("\",\n", 3);
    }

    /* title */
    if (node->type != LXB_GRAMMAR_NODE_GROUP) {
        lxb_grammar_indent_m(indent);
        lxb_grammar_cb_m("\"title\": ", 9);

        status = lxb_grammar_json_ast_node_value(node, func, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        lxb_grammar_cb_m(",\n", 2);
    }

    /* type */
    lxb_grammar_indent_m(indent);
    lxb_grammar_cb_m("\"type\": ", sizeof("\"type\": ") - 1);

    len = lexbor_conv_float_to_data((double) (uintptr_t) node->type,
                                    buf, sizeof(buf));
    lxb_grammar_cb_m(buf, len);
    lxb_grammar_cb_m(",\n", 2);

    /* combinator */
    if (node->type == LXB_GRAMMAR_NODE_GROUP
        || node->type == LXB_GRAMMAR_NODE_DECLARATION)
    {
        lxb_grammar_indent_m(indent);
        lxb_grammar_cb_m("\"combinator\": \"",
                         sizeof("\"combinator\": \"") - 1);

        status = lxb_grammar_json_ast_combinator(node, func, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        lxb_grammar_cb_m("\",\n", 3);
    }

    /* multiplier */
    static const lxb_char_t lxb_multiplier_b[] = "\"multiplier\": {\"start\": ";
    static const lxb_char_t lxb_multiplier_m[] = ", \"stop\": ";
    static const lxb_char_t lxb_multiplier_e[] = "}";

    lxb_grammar_indent_m(indent);

    if (node->multiplier.start == 1 && node->multiplier.stop == 1) {
        lxb_grammar_cb_m("\"multiplier\": null",
                         sizeof("\"multiplier\": null") - 1);
    }
    else {
        lxb_grammar_cb_m(lxb_multiplier_b, sizeof(lxb_multiplier_b) - 1);

        len = lexbor_conv_float_to_data((double) node->multiplier.start,
                                        buf, sizeof(buf));
        lxb_grammar_cb_m(buf, len);

        lxb_grammar_cb_m(lxb_multiplier_m, sizeof(lxb_multiplier_m) - 1);

        len = lexbor_conv_float_to_data((double) node->multiplier.stop,
                                        buf, sizeof(buf));
        lxb_grammar_cb_m(buf, len);

        lxb_grammar_cb_m(lxb_multiplier_e, sizeof(lxb_multiplier_e) - 1);
    }

    lxb_grammar_cb_m(",\n", 2);

    /* is_comma_separated */
    lxb_grammar_indent_m(indent);
    lxb_grammar_cb_m("\"is_comma_separated\": ",
                     sizeof("\"is_comma_separated\": ") - 1);

    if (node->is_comma) {
        lxb_grammar_cb_m("true", 4);
    }
    else {
        lxb_grammar_cb_m("false", 5);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_grammar_json_ast_node_value(const lxb_grammar_node_t *node,
                                lxb_grammar_serialize_cb_f func, void *ctx)
{
    size_t len;
    lxb_char_t buf[128];
    lxb_status_t status;
    const lxb_tag_data_t *tag_data;

    switch (node->type) {
        case LXB_GRAMMAR_NODE_STRING:
        case LXB_GRAMMAR_NODE_WHITESPACE:
        case LXB_GRAMMAR_NODE_DELIM:
        case LXB_GRAMMAR_NODE_UNQUOTED:
            lxb_grammar_cb_m("\"", 1);

            lxb_grammar_json_escape_cb_m(node->u.str.data, node->u.str.length);

            return func((lxb_char_t *) "\"", 1, ctx);

        case LXB_GRAMMAR_NODE_DECLARATION:
        case LXB_GRAMMAR_NODE_ELEMENT: {
            tag_data = lxb_tag_data_by_id(node->u.node->local_name);
            if (tag_data == NULL) {
                return LXB_STATUS_ERROR;
            }

            lxb_grammar_cb_m("\"", 1);

            status = lxb_grammar_json_escape(lexbor_hash_entry_str(&tag_data->entry),
                                             tag_data->entry.length, func, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            return func((lxb_char_t *) "\"", 1, ctx);
        }

        case LXB_GRAMMAR_NODE_NUMBER:
            len = lexbor_conv_float_to_data(node->u.num, buf,
                                            (sizeof(buf) / sizeof(lxb_char_t)));
            return func(buf, len, ctx);

        case LXB_GRAMMAR_NODE_LAST_ENTRY:
            lxb_grammar_cb_m("\"#end\"", 6);
            break;

        default:
            break;
    }

    return LXB_STATUS_OK;
}
