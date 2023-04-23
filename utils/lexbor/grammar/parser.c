/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/grammar/parser.h"
#include "lexbor/grammar/node.h"

#include "lexbor/core/utils.h"


static lxb_status_t
lxb_grammar_parser_serializer_callback(const lxb_char_t *data, size_t len,
                                       void *ctx);

static lxb_status_t
lxb_grammar_parser_state_begin(lxb_grammar_parser_t *parser,
                               lxb_grammar_token_t *token);

static lxb_status_t
lxb_grammar_parser_state_check_declaration(lxb_grammar_parser_t *parser,
                                           lxb_grammar_token_t *token);

static lxb_status_t
lxb_grammar_parser_state_skip_ws_declaration(lxb_grammar_parser_t *parser,
                                             lxb_grammar_token_t *token);

static lxb_status_t
lxb_grammar_parser_state_declaration(lxb_grammar_parser_t *parser,
                                     lxb_grammar_token_t *token);

static lxb_status_t
lxb_grammar_parser_state_combinator(lxb_grammar_parser_t *parser,
                                    lxb_grammar_token_t *token);

static lxb_status_t
lxb_grammar_parser_state_ws(lxb_grammar_parser_t *parser,
                            lxb_grammar_token_t *token);

static lxb_status_t
lxb_grammar_parser_state_declaration_mod(lxb_grammar_parser_t *parser,
                                         lxb_grammar_token_t *token);

static lxb_status_t
lxb_grammar_parser_state_declaration_mod_tw(lxb_grammar_parser_t *parser,
                                            lxb_grammar_token_t *token);


lxb_inline lxb_grammar_token_t *
lxb_grammar_parser_current_token(lxb_grammar_parser_t *parser)
{
    lexbor_array_t *tokens = parser->document->dom_document.user;

    if (parser->cur_token_id >= (tokens->length - 1)) {
        return NULL;
    }

    return tokens->list[ parser->cur_token_id ];
}

lxb_inline lxb_grammar_token_t *
lxb_grammar_parser_next_token(lxb_grammar_parser_t *parser)
{
    lexbor_array_t *tokens = parser->document->dom_document.user;

    if (parser->cur_token_id >= (tokens->length - 1)) {
        return NULL;
    }

    parser->cur_token_id++;

    return tokens->list[ parser->cur_token_id ];
}

lxb_inline void
lxb_grammar_parser_dec_token(lxb_grammar_parser_t *parser, size_t count)
{
    if ((parser->cur_token_id - count) <= 0) {
        parser->cur_token_id = 0;
    }

    parser->cur_token_id -= count;
}

lxb_grammar_parser_t *
lxb_grammar_parser_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_grammar_parser_t));
}

lxb_status_t
lxb_grammar_parser_init(lxb_grammar_parser_t *parser)
{
    if (parser == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    parser->index = 0;

    return LXB_STATUS_OK;
}

void
lxb_grammar_parser_clean(lxb_grammar_parser_t *parser)
{
    memset(parser, 0, sizeof(lxb_grammar_parser_t));
}

lxb_grammar_parser_t *
lxb_grammar_parser_destroy(lxb_grammar_parser_t *parser, bool self_destroy)
{
    if (parser == NULL) {
        return NULL;
    }

    if (self_destroy) {
        return lexbor_free(parser);
    }

    return parser;
}

lxb_grammar_node_t *
lxb_grammar_parser_process(lxb_grammar_parser_t *parser,
                           lxb_grammar_document_t *document)
{
    lxb_status_t status;
    lxb_grammar_token_t *token;

    parser->document = document;

    parser->root = lxb_grammar_node_create(parser, NULL, LXB_GRAMMAR_NODE_ROOT);
    if (parser->root == NULL) {
        return NULL;
    }

    parser->last_token = NULL;
    parser->last_error = NULL;

    parser->state = lxb_grammar_parser_state_begin;

    for (token = lxb_grammar_parser_current_token(parser); token != NULL;
         token = lxb_grammar_parser_next_token(parser))
    {
        status = parser->state(parser, token);
        if (status != LXB_STATUS_OK) {
            return NULL;
        }
    }

    return parser->root;
}

static lxb_status_t
lxb_grammar_parser_state_begin(lxb_grammar_parser_t *parser,
                               lxb_grammar_token_t *token)
{
    if (token->type == LXB_GRAMMAR_TOKEN_WHITESPACE) {
        return LXB_STATUS_OK;
    }
    else if (token->type != LXB_GRAMMAR_TOKEN_ELEMENT) {
        if (token->type == LXB_GRAMMAR_TOKEN_END_OF_FILE) {
            return LXB_STATUS_OK;
        }

        parser->last_token = token;
        parser->last_error = "Expected element of the declaration, "
                             "but received another token.";

        return LXB_STATUS_ERROR;
    }

    parser->node = lxb_grammar_node_create(parser, token,
                                           LXB_GRAMMAR_NODE_DECLARATION);
    if (parser->node == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    parser->group = parser->node;

    lxb_grammar_node_insert_child(parser->root, parser->node);

    parser->state = lxb_grammar_parser_state_check_declaration;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_grammar_parser_state_check_declaration(lxb_grammar_parser_t *parser,
                                           lxb_grammar_token_t *token)
{
    if (token->type == LXB_GRAMMAR_TOKEN_WHITESPACE) {
        if (token->flags & LXB_GRAMMAR_TOKEN_FLAGS_NEWLINE) {
            goto failed;
        }

        return LXB_STATUS_OK;
    }

    if (token->type != LXB_GRAMMAR_TOKEN_EQUALS) {
        goto failed;
    }

    parser->state = lxb_grammar_parser_state_skip_ws_declaration;

    return LXB_STATUS_OK;

failed:

    parser->last_token = token;
    parser->last_error = "Unexpected token in the declaration.";

    return LXB_STATUS_ERROR;
}

static lxb_status_t
lxb_grammar_parser_state_skip_ws_declaration(lxb_grammar_parser_t *parser,
                                             lxb_grammar_token_t *token)
{
    lxb_char_t *data;

    if (token->type == LXB_GRAMMAR_TOKEN_WHITESPACE) {
        data = token->u.str.data + (token->u.str.length - 1);

        if (*data == '\n' || *data == '\r') {
            parser->last_token = token;
            parser->last_error = "Whitespace token with new line "
                                 "in end of string.";

            return LXB_STATUS_ERROR;
        }

        return LXB_STATUS_OK;
    }
    else {
        lxb_grammar_parser_dec_token(parser, 1);
    }

    parser->state = lxb_grammar_parser_state_declaration;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_grammar_parser_rebuild_groups(lxb_grammar_parser_t *parser,
                                  lxb_grammar_combinator_t combinator)
{
    lxb_grammar_node_t *node, *group;

    if (parser->group->first_child == parser->group->last_child) {
        parser->group->combinator = combinator;

        return LXB_STATUS_OK;
    }

    if (parser->group->token == NULL && parser->group->parent->combinator == combinator) {
        parser->group = parser->group->parent;
        parser->node = parser->group->parent;
        return LXB_STATUS_OK;
    }

    group = lxb_grammar_node_create(parser, NULL,
                                    LXB_GRAMMAR_NODE_GROUP);
    if (group == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    if (parser->group->combinator == LXB_GRAMMAR_COMBINATOR_NORMAL) {
        node = parser->group->first_child;

        do {
            lxb_grammar_node_remove(node);
            lxb_grammar_node_insert_child(group, node);

            node = parser->group->first_child;
        }
        while (node != NULL);

        lxb_grammar_node_insert_child(parser->group, group);

//        parser->group = group;
//        parser->node = group;

        parser->group->combinator = combinator;

        return LXB_STATUS_OK;
    }

    group->combinator = combinator;

    node = parser->group->last_child;
    lxb_grammar_node_remove(node);

    lxb_grammar_node_insert_child(group, node);
    lxb_grammar_node_insert_child(parser->group, group);

    parser->group = group;
    parser->node = group;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_grammar_parser_state_declaration(lxb_grammar_parser_t *parser,
                                     lxb_grammar_token_t *token)
{
    lxb_grammar_node_t *group, *empty_group;

    switch (token->type) {
        case LXB_GRAMMAR_TOKEN_LEFT_BRACKET:
            parser->node = lxb_grammar_node_create(parser, token,
                                                   LXB_GRAMMAR_NODE_GROUP);
            if (parser->node == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            lxb_grammar_node_insert_child(parser->group, parser->node);
            parser->group = parser->node;

            parser->state = lxb_grammar_parser_state_ws;

            break;

        case LXB_GRAMMAR_TOKEN_RIGHT_BRACKET:
            empty_group = parser->group;
            group = NULL;

            while (empty_group && empty_group->token == NULL) {
                empty_group = empty_group->parent;
            }

            if (empty_group != NULL) {
                group = empty_group->parent;
            }

            if (group == NULL || empty_group == NULL
                || empty_group->token == NULL
                || (group->type != LXB_GRAMMAR_NODE_GROUP
                    && group->type != LXB_GRAMMAR_NODE_DECLARATION))
            {
                parser->last_token = token;
                parser->last_error = "Unexpected the right bracket token.";

                return LXB_STATUS_ERROR;
            }

            parser->to_mode = empty_group;
            parser->state = lxb_grammar_parser_state_declaration_mod;

            parser->group = group;
            parser->node = group;

            if (empty_group->first_child == NULL) {
                lxb_grammar_node_remove(empty_group);
            }

            break;

        case LXB_GRAMMAR_TOKEN_NUMBER:
            parser->node = lxb_grammar_node_create(parser, token,
                                                   LXB_GRAMMAR_NODE_NUMBER);
            goto insert_and_mode;

        case LXB_GRAMMAR_TOKEN_UNQUOTED:
            parser->node = lxb_grammar_node_create(parser, token,
                                                   LXB_GRAMMAR_NODE_UNQUOTED);
            goto insert_and_mode;

        case LXB_GRAMMAR_TOKEN_HASH:
        case LXB_GRAMMAR_TOKEN_DELIM:
            parser->node = lxb_grammar_node_create(parser, token,
                                                   LXB_GRAMMAR_NODE_DELIM);
            goto insert_and_mode;

        case LXB_GRAMMAR_TOKEN_STRING:
            parser->node = lxb_grammar_node_create(parser, token,
                                                   LXB_GRAMMAR_NODE_STRING);
            goto insert_and_mode;

        case LXB_GRAMMAR_TOKEN_ELEMENT:
            parser->node = lxb_grammar_node_create(parser, token,
                                                   LXB_GRAMMAR_NODE_ELEMENT);
            goto insert_and_mode;

        case LXB_GRAMMAR_TOKEN_END_OF_FILE:
            return LXB_STATUS_OK;

        default:
            parser->last_token = token;
            parser->last_error = "Unexpected token.";

            return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;

insert_and_mode:

    if (parser->node == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    lxb_grammar_node_insert_child(parser->group, parser->node);

    parser->to_mode = parser->node;
    parser->state = lxb_grammar_parser_state_declaration_mod;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_grammar_parser_state_combinator(lxb_grammar_parser_t *parser,
                                    lxb_grammar_token_t *token)
{
    lxb_char_t *data;
    lxb_status_t status;

    switch (token->type) {
        case LXB_GRAMMAR_TOKEN_WHITESPACE:
            data = token->u.str.data + (token->u.str.length - 1);

            if (*data == '\n' || *data == '\r') {
                parser->state = lxb_grammar_parser_state_begin;

                return LXB_STATUS_OK;
            }

            return LXB_STATUS_OK;

        case LXB_GRAMMAR_TOKEN_BAR:
            if (parser->group->combinator == LXB_GRAMMAR_COMBINATOR_ONE_OF) {
                break;
            }

            status = lxb_grammar_parser_rebuild_groups(parser,
                                                 LXB_GRAMMAR_COMBINATOR_ONE_OF);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            break;

        case LXB_GRAMMAR_TOKEN_DOUBLE_BAR:
            if (parser->group->combinator == LXB_GRAMMAR_COMBINATOR_OR) {
                break;
            }

            status = lxb_grammar_parser_rebuild_groups(parser,
                                                     LXB_GRAMMAR_COMBINATOR_OR);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            break;

        case LXB_GRAMMAR_TOKEN_AND:
            if (parser->group->combinator == LXB_GRAMMAR_COMBINATOR_AND) {
                break;
            }

            status = lxb_grammar_parser_rebuild_groups(parser,
                                                    LXB_GRAMMAR_COMBINATOR_AND);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            break;

//        case LXB_GRAMMAR_TOKEN_LEFT_BRACKET:
//        case LXB_GRAMMAR_TOKEN_RIGHT_BRACKET:
//            lxb_grammar_parser_dec_token(parser, 1);
//
//            parser->state = lxb_grammar_parser_state_declaration;
//
//            return LXB_STATUS_OK;

        case LXB_GRAMMAR_TOKEN_END_OF_FILE:
            parser->state = lxb_grammar_parser_state_declaration;

            return LXB_STATUS_OK;

        case LXB_GRAMMAR_TOKEN_LEFT_BRACKET:
        case LXB_GRAMMAR_TOKEN_RIGHT_BRACKET:
        default:
            if (parser->group->combinator != LXB_GRAMMAR_COMBINATOR_NORMAL) {
                status = lxb_grammar_parser_rebuild_groups(parser,
                                                 LXB_GRAMMAR_COMBINATOR_NORMAL);
                if (status != LXB_STATUS_OK) {
                    return status;
                }
            }

            lxb_grammar_parser_dec_token(parser, 1);

            parser->state = lxb_grammar_parser_state_declaration;

            return LXB_STATUS_OK;
    }

    parser->state = lxb_grammar_parser_state_ws;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_grammar_parser_state_ws(lxb_grammar_parser_t *parser,
                            lxb_grammar_token_t *token)
{
    lxb_char_t *data;

    parser->state = lxb_grammar_parser_state_declaration;

    if (token->type == LXB_GRAMMAR_TOKEN_WHITESPACE) {
        data = token->u.str.data + (token->u.str.length - 1);

        if (*data == '\n' || *data == '\r') {
            parser->state = lxb_grammar_parser_state_begin;

            return LXB_STATUS_OK;
        }

        return LXB_STATUS_OK;
    }

    if (token->type == LXB_GRAMMAR_TOKEN_END_OF_FILE) {
        parser->last_token = token;
        parser->last_error = "Unexpected end of file.";

        return LXB_STATUS_ERROR;
    }

    lxb_grammar_parser_dec_token(parser, 1);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_grammar_parser_state_declaration_mod(lxb_grammar_parser_t *parser,
                                         lxb_grammar_token_t *token)
{
    double num;

    /* -1 == infinity. */

    switch (token->type) {
        case LXB_GRAMMAR_TOKEN_ASTERISK:
            parser->to_mode->multiplier.start = 0;
            parser->to_mode->multiplier.stop = -1;
            break;

        case LXB_GRAMMAR_TOKEN_PLUS:
            parser->to_mode->multiplier.start = 1;
            parser->to_mode->multiplier.stop = -1;
            break;

        case LXB_GRAMMAR_TOKEN_EXCLAMATION:
            parser->to_mode->multiplier.start = 1;
            parser->to_mode->multiplier.stop = 0;
            break;

        case LXB_GRAMMAR_TOKEN_QUESTION:
            parser->to_mode->multiplier.start = 0;
            parser->to_mode->multiplier.stop = 1;
            break;

        case LXB_GRAMMAR_TOKEN_HASH:
            parser->to_mode->multiplier.start = 1;
            parser->to_mode->multiplier.stop = -1;

            parser->to_mode->is_comma = true;

            token = lxb_grammar_parser_next_token(parser);
            if (token == NULL) {
                return LXB_STATUS_ERROR;
            }

            switch (token->type) {
                case LXB_GRAMMAR_TOKEN_COUNT:
                    parser->to_mode->multiplier.start = token->u.count;
                    parser->to_mode->multiplier.stop = token->u.count;
                    break;

                case LXB_GRAMMAR_TOKEN_RANGE:
                    parser->to_mode->multiplier = token->u.period;
                    break;

                default:
                    lxb_grammar_parser_dec_token(parser, 1);
                    break;
            }

            break;

        case LXB_GRAMMAR_TOKEN_COUNT:
            parser->to_mode->multiplier.start = token->u.count;
            parser->to_mode->multiplier.stop = token->u.count;
            break;

        case LXB_GRAMMAR_TOKEN_RANGE:
            parser->to_mode->multiplier = token->u.period;
            break;

        case LXB_GRAMMAR_TOKEN_EXCLUDE_WS:
            if (parser->to_mode->skip_ws) {
                lxb_grammar_parser_dec_token(parser, 1);

                parser->state = lxb_grammar_parser_state_combinator;

                return LXB_STATUS_OK;
            }

            parser->to_mode->skip_ws = true;

            return LXB_STATUS_OK;

        case LXB_GRAMMAR_TOKEN_EXCLUDE_SORT:
            if (parser->to_mode->skip_sort) {
                lxb_grammar_parser_dec_token(parser, 1);

                parser->state = lxb_grammar_parser_state_combinator;

                return LXB_STATUS_OK;
            }

            parser->to_mode->skip_sort = true;

            return LXB_STATUS_OK;

        case LXB_GRAMMAR_TOKEN_DELIM:
            if (token->u.str.data[0] == '/') {
                token = lxb_grammar_parser_next_token(parser);
                if (token == NULL) {
                    return LXB_STATUS_ERROR;
                }

                if (token->type == LXB_GRAMMAR_TOKEN_NUMBER) {
                    num = token->u.num;

                    if (num < 0) {
                        num = 0;
                    }

                    parser->node->limit = (size_t) num;
                    break;
                }

                lxb_grammar_parser_dec_token(parser, 1);
            }

            /* fall through */

        default:
            lxb_grammar_parser_dec_token(parser, 1);

            parser->state = lxb_grammar_parser_state_combinator;

            break;
    }

    parser->state = lxb_grammar_parser_state_declaration_mod_tw;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_grammar_parser_state_declaration_mod_tw(lxb_grammar_parser_t *parser,
                                            lxb_grammar_token_t *token)
{
    /* -1 == infinity. */

    switch (token->type) {
        case LXB_GRAMMAR_TOKEN_EXCLUDE_WS:
            parser->to_mode->skip_ws = true;
            break;

        case LXB_GRAMMAR_TOKEN_EXCLUDE_SORT:
            parser->to_mode->skip_sort = true;
            break;

        default:
            lxb_grammar_parser_dec_token(parser, 1);
            break;
    }

    parser->state = lxb_grammar_parser_state_combinator;

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_grammar_parser_serializer_callback(const lxb_char_t *data, size_t len,
                                       void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}

void
lxb_grammar_parser_print_last_error(lxb_grammar_parser_t *parser)
{
    size_t len;
    const lxb_char_t *name;

    if (parser->last_error != NULL) {
        printf("%s\n", parser->last_error);
    }

    if (parser->last_token != NULL) {
        printf("Token");

        name = lxb_grammar_token_name(parser->last_token, &len);
        if (name != NULL) {
            printf(" (%.*s)", (int) len, (const char *) name);
        }

        printf(":\n");

        lxb_grammar_token_serialize(parser->last_token,
                                    lxb_grammar_parser_serializer_callback, NULL);
        printf("\n");
    }
}
