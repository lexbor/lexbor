/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_GRAMMAR_PARSER_H
#define LEXBOR_GRAMMAR_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/grammar/base.h"
#include "lexbor/grammar/node.h"

#include "lexbor/html/interfaces/document.h"


typedef lxb_status_t
(*lxb_grammar_parser_state_f)(lxb_grammar_parser_t *parser,
                              lxb_grammar_token_t *token);

struct lxb_grammar_parser {
    lxb_grammar_parser_state_f state;

    lxb_grammar_document_t     *document;
    lxb_grammar_node_t         *root;
    lxb_grammar_node_t         *node;
    lxb_grammar_node_t         *group;
    lxb_grammar_node_t         *to_mode;

    size_t                     cur_token_id;
    size_t                     index;

    lxb_grammar_token_t        *last_token;
    const char                 *last_error;
};


LXB_API lxb_grammar_parser_t *
lxb_grammar_parser_create(void);

LXB_API lxb_status_t
lxb_grammar_parser_init(lxb_grammar_parser_t *parser);

LXB_API void
lxb_grammar_parser_clean(lxb_grammar_parser_t *parser);

LXB_API lxb_grammar_parser_t *
lxb_grammar_parser_destroy(lxb_grammar_parser_t *parser, bool self_destroy);

LXB_API lxb_grammar_node_t *
lxb_grammar_parser_process(lxb_grammar_parser_t *parser,
                           lxb_grammar_document_t *document);

LXB_API void
lxb_grammar_parser_print_last_error(lxb_grammar_parser_t *parser);


/*
 * Inline functions
 */
lxb_inline lexbor_array_t *
lxb_grammar_parser_tokens(lxb_grammar_parser_t *parser)
{
    return parser->document->dom_document.user;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_GRAMMAR_PARSER_H */
