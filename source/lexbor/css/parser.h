/*

 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_CSS_PARSER_H
#define LEXBOR_CSS_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/css/log.h"
#include "lexbor/css/syntax/parser.h"
#include "lexbor/css/selectors/selectors.h"


#define lxb_css_parser_token_m(parser, token)                                  \
    do {                                                                       \
        token = lxb_css_syntax_token((parser)->tkz);                           \
        if (token == NULL) {                                                   \
            return lxb_css_parser_fail((parser), (parser)->tkz->status);       \
        }                                                                      \
    }                                                                          \
    while (false)

#define lxb_css_parser_token_next_m(parser, token)                             \
    do {                                                                       \
        token = lxb_css_syntax_token_next((parser)->tkz);                      \
        if (token == NULL) {                                                   \
            return lxb_css_parser_fail((parser), (parser)->tkz->status);       \
        }                                                                      \
    }                                                                          \
    while (false)

#define lxb_css_parser_token_wo_ws_m(parser, token)                            \
    do {                                                                       \
        token = lxb_css_syntax_token((parser)->tkz);                           \
        if (token == NULL) {                                                   \
            return lxb_css_parser_fail((parser), (parser)->tkz->status);       \
        }                                                                      \
                                                                               \
        if (token->type == LXB_CSS_SYNTAX_TOKEN_WHITESPACE) {                  \
            lxb_css_syntax_token_consume(parser->tkz);                         \
            token = lxb_css_syntax_token((parser)->tkz);                       \
            if (token == NULL) {                                               \
                return lxb_css_parser_fail((parser), (parser)->tkz->status);   \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    while (false)

#define lxb_css_parser_token_status_m(parser, token)                           \
    do {                                                                       \
        if ((token = lxb_css_syntax_token((parser)->tkz)) == NULL) {           \
            return parser->tkz->status;                                        \
        }                                                                      \
    }                                                                          \
    while (false)

#define lxb_css_parser_token_status_next_m(parser, token)                      \
    do {                                                                       \
        token = lxb_css_syntax_token_next((parser)->tkz);                      \
        if (token == NULL) {                                                   \
            return parser->tkz->status;                                        \
        }                                                                      \
    }                                                                          \
    while (false)


#define lxb_css_parser_token_status_wo_ws_m(parser, token)                     \
    do {                                                                       \
        if ((token = lxb_css_syntax_token((parser)->tkz)) == NULL) {           \
            return parser->tkz->status;                                        \
        }                                                                      \
                                                                               \
        if (token->type == LXB_CSS_SYNTAX_TOKEN_WHITESPACE) {                  \
            lxb_css_syntax_token_consume(parser->tkz);                         \
            if ((token = lxb_css_syntax_token((parser)->tkz)) == NULL) {       \
                return parser->tkz->status;                                    \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    while (false)

#define lxb_css_parser_string_dup_m(parser, token, _str, mraw)                 \
    do {                                                                       \
        if (lxb_css_syntax_token_string_dup(lxb_css_syntax_token_string(token),\
                                            (_str), (mraw)) != LXB_STATUS_OK)  \
        {                                                                      \
            return lxb_css_parser_fail((parser),                               \
                                       LXB_STATUS_ERROR_MEMORY_ALLOCATION);    \
        }                                                                      \
    }                                                                          \
    while (false)


enum {
    LXB_CSS_SYNTAX_PARSER_ERROR_UNDEF = 0x0000,
    /* eof-in-at-rule */
    LXB_CSS_SYNTAX_PARSER_ERROR_EOINATRU,
    /* eof-in-qualified-rule */
    LXB_CSS_SYNTAX_PARSER_ERROR_EOINQURU,
    /* eof-in-simple-block */
    LXB_CSS_SYNTAX_PARSER_ERROR_EOINSIBL,
    /* eof-in-function */
    LXB_CSS_SYNTAX_PARSER_ERROR_EOINFU,
    /* eof-before-parse-rule */
    LXB_CSS_SYNTAX_PARSER_ERROR_EOBEPARU,
    /* unexpected-token-after-parse-rule */
    LXB_CSS_SYNTAX_PARSER_ERROR_UNTOAFPARU,
    /* eof-before-parse-component-value */
    LXB_CSS_SYNTAX_PARSER_ERROR_EOBEPACOVA,
    /* unexpected-token-after-parse-component-value */
    LXB_CSS_SYNTAX_PARSER_ERROR_UNTOAFPACOVA,
    /* unexpected-token-in-declaration */
    LXB_CSS_SYNTAX_PARSER_ERROR_UNTOINDE,
};

typedef enum {
    LXB_CSS_PARSER_CLEAN = 0,
    LXB_CSS_PARSER_RUN,
    LXB_CSS_PARSER_STOP,
    LXB_CSS_PARSER_END
}
lxb_css_parser_stage_t;

struct lxb_css_parser {
    lxb_css_parser_state_f      state;
    void                        *context;

    /* Modules */
    lxb_css_syntax_tokenizer_t  *tkz;
    lxb_css_selectors_t         *selectors;

    /* Memory for all structures. */
    lexbor_mraw_t               *mraw;

    /* Stack */
    lxb_css_parser_stack_t      *stack_begin;
    lxb_css_parser_stack_t      *stack_end;
    lxb_css_parser_stack_t      *stack;

    /* Types */
    lxb_css_syntax_token_type_t *types_begin;
    lxb_css_syntax_token_type_t *types_end;
    lxb_css_syntax_token_type_t *types_pos;

    lxb_css_log_t               *log;

    lxb_css_parser_stage_t      stage;

    bool                        loop;
    bool                        top_level;
    bool                        my_mraw;
    bool                        my_tkz;
    lxb_status_t                status;
};

struct lxb_css_parser_stack {
    lxb_css_parser_state_f state;
    void                   *context;
    bool                   required_stop;
};

struct lxb_css_parser_error {
    lexbor_str_t message;
};


LXB_API lxb_css_parser_t *
lxb_css_parser_create(void);

LXB_API lxb_status_t
lxb_css_parser_init(lxb_css_parser_t *parser,
                    lxb_css_syntax_tokenizer_t *tkz, lexbor_mraw_t *mraw);

LXB_API void
lxb_css_parser_clean(lxb_css_parser_t *parser);

LXB_API lxb_css_parser_t *
lxb_css_parser_destroy(lxb_css_parser_t *parser, bool self_destroy);

LXB_API lxb_css_parser_stack_t *
lxb_css_parser_stack_push(lxb_css_parser_t *parser,
                          lxb_css_parser_state_f state, void *context, bool stop);

LXB_API lxb_status_t
lxb_css_parser_run(lxb_css_parser_t *parser,
                   lxb_css_parser_state_f state, void *context);

LXB_API bool
lxb_css_parser_stop(lxb_css_parser_t *parser);

LXB_API bool
lxb_css_parser_fail(lxb_css_parser_t *parser, lxb_status_t status);

LXB_API bool
lxb_css_parser_unexpected(lxb_css_parser_t *parser);

LXB_API lxb_status_t
lxb_css_parser_unexpected_status(lxb_css_parser_t *parser);

LXB_API bool
lxb_css_parser_unexpected_data(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token);

LXB_API lxb_status_t
lxb_css_parser_unexpected_data_status(lxb_css_parser_t *parser,
                                      const lxb_css_syntax_token_t *token);

LXB_API bool
lxb_css_parser_memory_fail(lxb_css_parser_t *parser);

LXB_API lxb_css_syntax_token_t *
lxb_css_parser_find_close(lxb_css_parser_t *parser, lxb_css_syntax_token_t *token,
                          const lxb_char_t *opt_data, lxb_css_syntax_token_type_t type,
                          lxb_css_syntax_token_type_t stop);

LXB_API lxb_css_syntax_token_t *
lxb_css_parser_find_close_deep(lxb_css_parser_t *parser,
                               lxb_css_syntax_token_t *token,
                               lxb_css_syntax_token_type_t type, size_t *deep);

/*
 * Inline functions
 */
lxb_inline lxb_status_t
lxb_css_parser_status(lxb_css_parser_t *parser)
{
    return parser->status;
}

lxb_inline lxb_css_selectors_t *
lxb_css_parser_selectors(lxb_css_parser_t *parser)
{
    return parser->selectors;
}

lxb_inline void
lxb_css_parser_selectors_set(lxb_css_parser_t *parser,
                             lxb_css_selectors_t *selectors)
{
    parser->selectors = selectors;
}


lxb_inline bool
lxb_css_parser_status_is_unexpected_data(lxb_css_parser_t *parser)
{
    return parser->status == LXB_STATUS_ERROR_UNEXPECTED_DATA;
}

lxb_inline const lxb_char_t *
lxb_css_parser_buffer(lxb_css_parser_t *parser, size_t *length)
{
    if (length != NULL) {
        *length = parser->tkz->in_end - parser->tkz->in_begin;
    }

    return parser->tkz->in_begin;
}

lxb_inline void
lxb_css_parser_buffer_set(lxb_css_parser_t *parser,
                          const lxb_char_t *data, size_t length)
{
    lxb_css_syntax_tokenizer_buffer_set(parser->tkz, data, length);
}

lxb_inline lxb_css_parser_state_f
lxb_css_parser_state(lxb_css_parser_t *parser)
{
    return parser->state;
}

lxb_inline void
lxb_css_parser_state_set(lxb_css_parser_t *parser, lxb_css_parser_state_f state)
{
    parser->state = state;
}

lxb_inline void *
lxb_css_parser_context(lxb_css_parser_t *parser)
{
    return parser->context;
}

lxb_inline void
lxb_css_parser_context_set(lxb_css_parser_t *parser, void *context)
{
    parser->context = context;
}

lxb_inline lxb_css_parser_stack_t *
lxb_css_parser_stack_pop(lxb_css_parser_t *parser)
{
    lxb_css_parser_stack_t *entry = --parser->stack;

    parser->state = entry->state;
    parser->context = entry->context;

    return entry;
}

lxb_inline lxb_css_parser_stack_t *
lxb_css_parser_stack_to_stop(lxb_css_parser_t *parser)
{
    lxb_css_parser_stack_t *entry = --parser->stack;

    while (!entry->required_stop) {
        entry--;
    }

    parser->state = entry->state;
    parser->context = entry->context;

    parser->stack = entry;

    return entry;
}

lxb_inline void
lxb_css_parser_stack_clean(lxb_css_parser_t *parser)
{
    parser->stack = parser->stack_begin;
}

lxb_inline lxb_css_parser_stack_t *
lxb_css_parser_stack_current(lxb_css_parser_t *parser)
{
    return parser->stack - 1;
}

lxb_inline void
lxb_css_parser_stack_set(lxb_css_parser_stack_t *stack,
                         lxb_css_parser_state_f state, void *context)
{
    stack->state = state;
    stack->context = context;
}

lxb_inline void
lxb_css_parser_stack_up(lxb_css_parser_t *parser)
{
    parser->stack++;
}

lxb_inline void
lxb_css_parser_stack_down(lxb_css_parser_t *parser)
{
    parser->stack--;
}

lxb_inline lxb_css_log_t *
lxb_css_parser_log(lxb_css_parser_t *parser)
{
    return parser->log;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_CSS_PARSER_H */
