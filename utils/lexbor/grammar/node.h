/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_GRAMMAR_NODE_H
#define LEXBOR_GRAMMAR_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/grammar/base.h"
#include "lexbor/grammar/token.h"


typedef enum {
    LXB_GRAMMAR_NODE_UNDEF = 0x00,
    LXB_GRAMMAR_NODE_ROOT,

    /* For [...] */
    LXB_GRAMMAR_NODE_GROUP,

    /* In node->u.node */
    LXB_GRAMMAR_NODE_DECLARATION,
    LXB_GRAMMAR_NODE_ELEMENT,

    /* In node->u.num */
    LXB_GRAMMAR_NODE_NUMBER,

    /* All in node->u.str */
    LXB_GRAMMAR_NODE_STRING,
    LXB_GRAMMAR_NODE_WHITESPACE,
    LXB_GRAMMAR_NODE_DELIM,
    LXB_GRAMMAR_NODE_UNQUOTED,

    /* Used for last entry in tree */
    LXB_GRAMMAR_NODE_LAST_ENTRY
}
lxb_grammar_node_type_t;

typedef enum {
    LXB_GRAMMAR_COMBINATOR_NORMAL = 0x00,
    LXB_GRAMMAR_COMBINATOR_AND,          /* && -- all of which must occur, in any order */
    LXB_GRAMMAR_COMBINATOR_OR,           /* || -- one or more of them must occur, in any order */
    LXB_GRAMMAR_COMBINATOR_ONE_OF,       /* |  -- exactly one of them must occur */
}
lxb_grammar_combinator_t;

struct lxb_grammar_node {
    lxb_grammar_node_type_t  type;
    size_t                   index;

    union lxb_grammar_node_u {
        double         num;
        lexbor_str_t   str;
        lxb_dom_node_t *node;
    }
    u;

    lxb_grammar_combinator_t combinator;
    lxb_grammar_period_t     multiplier;
    bool                     is_comma;
    bool                     skip_ws;
    bool                     skip_sort;
    size_t                   limit;

    lxb_grammar_token_t      *token;
    lxb_grammar_document_t   *document;

    lxb_grammar_node_t       *next;
    lxb_grammar_node_t       *prev;
    lxb_grammar_node_t       *first_child;
    lxb_grammar_node_t       *last_child;
    lxb_grammar_node_t       *parent;
};


LXB_API lxb_grammar_node_t *
lxb_grammar_node_create(lxb_grammar_parser_t *parser, lxb_grammar_token_t *token,
                        lxb_grammar_node_type_t type);

LXB_API void
lxb_grammar_node_clean(lxb_grammar_node_t *node);

LXB_API lxb_grammar_node_t *
lxb_grammar_node_destroy(lxb_grammar_node_t *node);


LXB_API void
lxb_grammar_node_insert_child(lxb_grammar_node_t *to, lxb_grammar_node_t *node);

LXB_API void
lxb_grammar_node_insert_before(lxb_grammar_node_t *to, lxb_grammar_node_t *node);

LXB_API void
lxb_grammar_node_insert_after(lxb_grammar_node_t *to, lxb_grammar_node_t *node);

LXB_API void
lxb_grammar_node_remove(lxb_grammar_node_t *node);

LXB_API lxb_status_t
lxb_grammar_node_serialize_deep(const lxb_grammar_node_t *root,
                                lxb_grammar_serialize_cb_f func, void *ctx);

LXB_API lxb_status_t
lxb_grammar_node_serialize(const lxb_grammar_node_t *node,
                           lxb_grammar_serialize_cb_f func, void *ctx);

LXB_API lxb_status_t
lxb_grammar_node_serialize_ast(const lxb_grammar_node_t *root,
                               lxb_grammar_serialize_cb_f func, void *ctx);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_GRAMMAR_NODE_H */
