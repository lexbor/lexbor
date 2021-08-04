/*
 * Copyright (C) 2020-2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/css/parser.h"
#include "lexbor/css/selectors/state.h"
#include "lexbor/css/selectors/selectors.h"
#include "lexbor/css/selectors/pseudo.h"
#include "lexbor/css/selectors/pseudo_const.h"


static const char lxb_css_selectors_module_name[] = "Selectors";


#define lxb_css_selectors_state_string_dup_m(selectors, name)                  \
    do {                                                                       \
        (status) = lxb_css_syntax_token_string_dup(                            \
                    lxb_css_syntax_token_string(token), (name),                \
                    (selectors)->memory->mraw);                                \
        if ((status) != LXB_STATUS_OK) {                                       \
            return (status);                                                   \
        }                                                                      \
    }                                                                          \
    while (false)

#define lxb_css_selectors_state_append(parser, selectors, selector)            \
    do {                                                                       \
        (selector) = lxb_css_selector_create((selectors)->list_last);          \
        if ((selector) == NULL) {                                              \
            return lxb_css_parser_memory_fail(parser);                         \
        }                                                                      \
                                                                               \
        lxb_css_selectors_append_next((selectors), (selector));                \
                                                                               \
        (selector)->combinator = (selectors)->combinator;                      \
        (selectors)->combinator = LXB_CSS_SELECTOR_COMBINATOR_CLOSE;           \
    }                                                                          \
    while (false)

#define lxb_css_selectors_state_list_append(parser, selectors, list)           \
    do {                                                                       \
        (list) = lxb_css_selector_list_create((selectors)->memory);            \
        if ((list) == NULL) {                                                  \
            return lxb_css_parser_memory_fail(parser);                         \
        }                                                                      \
                                                                               \
        lxb_css_selectors_list_append_next((selectors), (list));               \
                                                                               \
        (list)->parent = selectors->parent;                                    \
    }                                                                          \
    while (false)


lxb_css_syntax_token_t *
lxb_css_selectors_close_parenthesis(lxb_css_parser_t *parser,
                                    lxb_css_syntax_token_t *token);

static bool
lxb_css_selectors_state_complex_list_end(lxb_css_parser_t *parser,
                                         lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_relative_list_end(lxb_css_parser_t *parser,
                                          lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_complex_end(lxb_css_parser_t *parser,
                                    lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_compound_list_end(lxb_css_parser_t *parser,
                                          lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_compound_handler(lxb_css_parser_t *parser,
                                         lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_compound_sub(lxb_css_parser_t *parser,
                                     lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_compound_pseudo(lxb_css_parser_t *parser,
                                        lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_simple_list_end(lxb_css_parser_t *parser,
                                        lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_simple_handler(lxb_css_parser_t *parser,
                                       lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
lxb_css_selectors_state_hash(lxb_css_parser_t *parser,
                             lxb_css_syntax_token_t *token);

static lxb_status_t
lxb_css_selectors_state_class(lxb_css_parser_t *parser,
                              lxb_css_syntax_token_t *token);

static lxb_status_t
lxb_css_selectors_state_element_ns(lxb_css_parser_t *parser,
                                   lxb_css_syntax_token_t *token);

static lxb_status_t
lxb_css_selectors_state_element(lxb_css_parser_t *parser,
                                lxb_css_syntax_token_t *token);

static lxb_status_t
lxb_css_selectors_state_attribute(lxb_css_parser_t *parser);

static lxb_status_t
lxb_css_selectors_state_ns(lxb_css_parser_t *parser,
                           lxb_css_selector_t *selector);

static lxb_status_t
lxb_css_selectors_state_ns_ident(lxb_css_parser_t *parser,
                                 lxb_css_selector_t *selector);

static lxb_status_t
lxb_css_selectors_state_pseudo_class(lxb_css_parser_t *parser,
                                     lxb_css_syntax_token_t *token);

static lxb_status_t
lxb_css_selectors_state_pseudo_class_function(lxb_css_parser_t *parser,
                                              lxb_css_syntax_token_t *token,
                                              lxb_css_parser_state_f success);

static lxb_status_t
lxb_css_selectors_state_pseudo_element(lxb_css_parser_t *parser,
                                       lxb_css_syntax_token_t *token);

static lxb_status_t
lxb_css_selectors_state_pseudo_element_function(lxb_css_parser_t *parser,
                                                lxb_css_syntax_token_t *token,
                                                lxb_css_parser_state_f success);

static bool
lxb_css_selectors_state_forgiving_cb(lxb_css_parser_t *parser,
                                     lxb_css_syntax_token_t *token, void *ctx,
                                     lxb_css_parser_state_f state);

static void
lxb_css_selectors_state_restore_parent(lxb_css_selectors_t *selectors,
                                       lxb_css_selector_list_t *last);

static bool
lxb_css_selectors_state_list_end(lxb_css_parser_t *parser,
                                 lxb_css_syntax_token_t *token,
                                 lxb_css_parser_state_f state);


lxb_inline bool
lxb_css_selectors_done(lxb_css_parser_t *parser)
{
    (void) lxb_css_parser_stack_pop(parser);

    return true;
}


/*
 * <complex-selector-list>
 */
bool
lxb_css_selectors_state_complex_list(lxb_css_parser_t *parser,
                                     lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_parser_stack_t *stack;

    stack = lxb_css_parser_stack_push(parser, lxb_css_selectors_state_complex_list_end,
                                      ctx, false);
    if (stack == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    parser->state = lxb_css_selectors_state_complex;

    return false;
}

static bool
lxb_css_selectors_state_complex_list_end(lxb_css_parser_t *parser,
                                         lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_list_end(parser, token,
                                            lxb_css_selectors_state_complex);
}

/*
 * <relative-selector-list>
 */
bool
lxb_css_selectors_state_relative_list(lxb_css_parser_t *parser,
                                      lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_parser_stack_t *stack;

    stack = lxb_css_parser_stack_push(parser, lxb_css_selectors_state_relative_list_end,
                                      ctx, false);
    if (stack == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    parser->state = lxb_css_selectors_state_relative;

    return false;
}

static bool
lxb_css_selectors_state_relative_list_end(lxb_css_parser_t *parser,
                                          lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_list_end(parser, token,
                                            lxb_css_selectors_state_relative);
}

/*
 * <relative-selector>
 */
bool
lxb_css_selectors_state_relative(lxb_css_parser_t *parser,
                                 lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_parser_stack_t *stack;
    lxb_css_selectors_t *selectors = parser->selectors;

    /* <combinator> */

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_WHITESPACE:
            lxb_css_syntax_token_consume(parser->tkz);
            selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT;
            return true;

        case LXB_CSS_SYNTAX_TOKEN_DELIM:
            switch (lxb_css_syntax_token_delim_char(token)) {
                case '>':
                    selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_CHILD;
                    break;

                case '+':
                    selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_SIBLING;
                    break;

                case '~':
                    selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_FOLLOWING;
                    break;

                case '|':
                    lxb_css_parser_token_next_m(parser, token);

                    if (token->type == LXB_CSS_SYNTAX_TOKEN_DELIM
                        && lxb_css_syntax_token_delim_char(token) == '|')
                    {
                        lxb_css_syntax_token_consume(parser->tkz);
                        selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_CELL;
                        break;
                    }

                    goto done;

                default:
                    goto done;
            }

            break;

        default:
            goto done;
    }

    lxb_css_syntax_token_consume(parser->tkz);

done:

    stack = lxb_css_parser_stack_push(parser, lxb_css_selectors_state_complex_end,
                                      ctx, false);
    if (stack == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    parser->state = lxb_css_selectors_state_compound;

    return true;
}

/*
 * <complex-selector>
 */
bool
lxb_css_selectors_state_complex(lxb_css_parser_t *parser,
                                 lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_parser_stack_t *stack;

    stack = lxb_css_parser_stack_push(parser, lxb_css_selectors_state_complex_end,
                                      ctx, false);
    if (stack == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    parser->state = lxb_css_selectors_state_compound;

    return false;
}

static bool
lxb_css_selectors_state_complex_end(lxb_css_parser_t *parser,
                                    lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_selectors_t *selectors = parser->selectors;

    /* <combinator> */

again:

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_WHITESPACE:
            lxb_css_syntax_token_consume(parser->tkz);

            selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT;

            lxb_css_parser_token_m(parser, token);
            goto again;

        case LXB_CSS_SYNTAX_TOKEN__EOF:
            return lxb_css_selectors_done(parser);

        case LXB_CSS_SYNTAX_TOKEN_DELIM:
            switch (lxb_css_syntax_token_delim_char(token)) {
                case '>':
                    selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_CHILD;
                    break;

                case '+':
                    selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_SIBLING;
                    break;

                case '~':
                    selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_FOLLOWING;
                    break;

                case '|':
                    lxb_css_parser_token_next_m(parser, token);

                    if (token->type == LXB_CSS_SYNTAX_TOKEN_DELIM
                        && lxb_css_syntax_token_delim_char(token) == '|')
                    {
                        lxb_css_syntax_token_consume(parser->tkz);
                        selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_CELL;
                        break;
                    }

                    goto done;

                default:
                    if (selectors->combinator != LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT) {
                        return lxb_css_selectors_done(parser);
                    }

                    goto done;
            }

            break;

        case LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS:
        case LXB_CSS_SYNTAX_TOKEN_COMMA:
            return lxb_css_selectors_done(parser);

        default:
            if (selectors->combinator != LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT) {
                return lxb_css_selectors_done(parser);
            }

            goto done;
    }

    lxb_css_syntax_token_consume(parser->tkz);

done:

    lxb_css_parser_stack_up(parser);

    parser->state = lxb_css_selectors_state_compound_handler;

    return true;
}

/*
 * <compound-selector-list>
 */
bool
lxb_css_selectors_state_compound_list(lxb_css_parser_t *parser,
                                      lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_parser_stack_t *stack;

    stack = lxb_css_parser_stack_push(parser, lxb_css_selectors_state_compound_list_end,
                                      ctx, false);
    if (stack == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    parser->state = lxb_css_selectors_state_compound;

    return false;
}

static bool
lxb_css_selectors_state_compound_list_end(lxb_css_parser_t *parser,
                                          lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_list_end(parser, token,
                                            lxb_css_selectors_state_compound);
}

/*
 *
 * <compound-selector>
 */
bool
lxb_css_selectors_state_compound(lxb_css_parser_t *parser,
                                 lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_selector_list_t *list;

    lxb_css_selectors_state_list_append(parser, parser->selectors, list);

    parser->state = lxb_css_selectors_state_compound_handler;

    return false;
}

static bool
lxb_css_selectors_state_compound_handler(lxb_css_parser_t *parser,
                                         lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_status_t status;
    lxb_css_selectors_t *selectors;

again:

    parser->state = lxb_css_selectors_state_compound_sub;

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_HASH:
            status = lxb_css_selectors_state_hash(parser, token);
            break;

        case LXB_CSS_SYNTAX_TOKEN_DELIM:
            switch (lxb_css_syntax_token_delim_char(token)) {
                case '.':
                    lxb_css_syntax_token_consume(parser->tkz);
                    status = lxb_css_selectors_state_class(parser, token);
                    break;

                case '|':
                case '*':
                    lxb_css_syntax_token_consume(parser->tkz);
                    status = lxb_css_selectors_state_element_ns(parser, token);
                    break;

                default:
                    return lxb_css_parser_unexpected(parser);
            }

            break;

        case LXB_CSS_SYNTAX_TOKEN_IDENT:
            status = lxb_css_selectors_state_element(parser, token);
            break;

        case LXB_CSS_SYNTAX_TOKEN_LS_BRACKET:
            lxb_css_syntax_token_consume(parser->tkz);
            status = lxb_css_selectors_state_attribute(parser);
            break;

        case LXB_CSS_SYNTAX_TOKEN_COLON:
            lxb_css_syntax_token_consume(parser->tkz);
            lxb_css_parser_token_m(parser, token);

            if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
                status = lxb_css_selectors_state_pseudo_class(parser, token);
                break;
            }
            else if (token->type == LXB_CSS_SYNTAX_TOKEN_COLON) {
                lxb_css_syntax_token_consume(parser->tkz);
                lxb_css_parser_token_m(parser, token);

                if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
                    parser->state = lxb_css_selectors_state_compound_pseudo;
                    status = lxb_css_selectors_state_pseudo_element(parser, token);
                    break;
                }
                else if (token->type != LXB_CSS_SYNTAX_TOKEN_FUNCTION) {
                    return lxb_css_parser_unexpected(parser);
                }

                status = lxb_css_selectors_state_pseudo_element_function(parser, token,
                                               lxb_css_selectors_state_compound_pseudo);
                break;
            }
            else if (token->type != LXB_CSS_SYNTAX_TOKEN_FUNCTION) {
                return lxb_css_parser_unexpected(parser);
            }

            status = lxb_css_selectors_state_pseudo_class_function(parser, token,
                                            lxb_css_selectors_state_compound_sub);
            break;

        case LXB_CSS_SYNTAX_TOKEN_WHITESPACE:
            lxb_css_syntax_token_consume(parser->tkz);
            lxb_css_parser_token_m(parser, token);
            goto again;

        case LXB_CSS_SYNTAX_TOKEN__EOF:
            selectors = parser->selectors;

            if (selectors->combinator > LXB_CSS_SELECTOR_COMBINATOR_CLOSE
                || selectors->list_last->first == NULL)
            {
                return lxb_css_parser_unexpected(parser);
            }

            return lxb_css_selectors_done(parser);

        default:
            return lxb_css_parser_unexpected(parser);
    }

    if (status == LXB_STATUS_ERROR_MEMORY_ALLOCATION) {
        return lxb_css_parser_memory_fail(parser);
    }

    return true;
}

static bool
lxb_css_selectors_state_compound_sub(lxb_css_parser_t *parser,
                                     lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_status_t status;

    /* <subclass-selector> */

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_HASH:
            status = lxb_css_selectors_state_hash(parser, token);
            break;

        case LXB_CSS_SYNTAX_TOKEN_DELIM:
            switch (lxb_css_syntax_token_delim_char(token)) {
                case '.':
                    lxb_css_syntax_token_consume(parser->tkz);
                    status = lxb_css_selectors_state_class(parser, token);
                    break;

                default:
                    return lxb_css_selectors_done(parser);
            }

            break;

        case LXB_CSS_SYNTAX_TOKEN_LS_BRACKET:
            lxb_css_syntax_token_consume(parser->tkz);
            status = lxb_css_selectors_state_attribute(parser);
            break;

        case LXB_CSS_SYNTAX_TOKEN_COLON:
            lxb_css_syntax_token_consume(parser->tkz);
            lxb_css_parser_token_m(parser, token);

            if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
                status = lxb_css_selectors_state_pseudo_class(parser, token);
                break;
            }
            else if (token->type == LXB_CSS_SYNTAX_TOKEN_COLON) {
                lxb_css_syntax_token_consume(parser->tkz);
                lxb_css_parser_token_m(parser, token);

                if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
                    parser->state = lxb_css_selectors_state_compound_pseudo;
                    status = lxb_css_selectors_state_pseudo_element(parser, token);
                    break;
                }
                else if (token->type != LXB_CSS_SYNTAX_TOKEN_FUNCTION) {
                    return lxb_css_parser_unexpected(parser);
                }

                status = lxb_css_selectors_state_pseudo_element_function(parser, token,
                                               lxb_css_selectors_state_compound_pseudo);
                break;
            }
            else if (token->type != LXB_CSS_SYNTAX_TOKEN_FUNCTION) {
                return lxb_css_parser_unexpected(parser);
            }

            status = lxb_css_selectors_state_pseudo_class_function(parser, token,
                                            lxb_css_selectors_state_compound_sub);
            break;

        default:
            return lxb_css_selectors_done(parser);
    }

    if (status == LXB_STATUS_ERROR_MEMORY_ALLOCATION) {
        return lxb_css_parser_memory_fail(parser);
    }

    return true;
}

static bool
lxb_css_selectors_state_compound_pseudo(lxb_css_parser_t *parser,
                                        lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_status_t status;

    if (token->type != LXB_CSS_SYNTAX_TOKEN_COLON) {
        return lxb_css_selectors_done(parser);
    }

    lxb_css_syntax_token_consume(parser->tkz);
    lxb_css_parser_token_m(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
        status = lxb_css_selectors_state_pseudo_class(parser, token);
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_COLON) {
        lxb_css_syntax_token_consume(parser->tkz);
        lxb_css_parser_token_m(parser, token);

        if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
            status = lxb_css_selectors_state_pseudo_element(parser, token);
        }
        else if (token->type == LXB_CSS_SYNTAX_TOKEN_FUNCTION) {
            status = lxb_css_selectors_state_pseudo_element_function(parser, token,
                                           lxb_css_selectors_state_compound_pseudo);
        }
        else {
            return lxb_css_parser_unexpected(parser);
        }
    }
    else if (token->type != LXB_CSS_SYNTAX_TOKEN_FUNCTION) {
        return lxb_css_parser_unexpected(parser);
    }
    else {
        status = lxb_css_selectors_state_pseudo_class_function(parser, token,
                                       lxb_css_selectors_state_compound_pseudo);
    }

    if (status == LXB_STATUS_ERROR_MEMORY_ALLOCATION) {
        return lxb_css_parser_memory_fail(parser);
    }

    return true;
}

/*
 * <simple-selector-list>
 */
bool
lxb_css_selectors_state_simple_list(lxb_css_parser_t *parser,
                                    lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_parser_stack_t *stack;

    stack = lxb_css_parser_stack_push(parser, lxb_css_selectors_state_simple_list_end,
                                      ctx, false);
    if (stack == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    parser->state = lxb_css_selectors_state_simple;

    return false;
}

static bool
lxb_css_selectors_state_simple_list_end(lxb_css_parser_t *parser,
                                        lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_list_end(parser, token,
                                            lxb_css_selectors_state_simple);
}

/*
 * <simple-selector>
 */
bool
lxb_css_selectors_state_simple(lxb_css_parser_t *parser,
                               lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_selector_list_t *list;

    lxb_css_selectors_state_list_append(parser, parser->selectors, list);

    parser->state = lxb_css_selectors_state_simple_handler;

    return false;
}

static bool
lxb_css_selectors_state_simple_handler(lxb_css_parser_t *parser,
                                       lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_status_t status;

again:

    parser->state = lxb_css_selectors_state_compound_sub;

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_HASH:
            status = lxb_css_selectors_state_hash(parser, token);
            break;

        case LXB_CSS_SYNTAX_TOKEN_DELIM:
            switch (lxb_css_syntax_token_delim_char(token)) {
                case '.':
                    lxb_css_syntax_token_consume(parser->tkz);
                    status = lxb_css_selectors_state_class(parser, token);
                    break;

                case '|':
                case '*':
                    lxb_css_syntax_token_consume(parser->tkz);
                    status = lxb_css_selectors_state_element_ns(parser, token);
                    break;

                default:
                    return lxb_css_parser_unexpected(parser);
            }

            break;

        case LXB_CSS_SYNTAX_TOKEN_IDENT:
            status = lxb_css_selectors_state_element(parser, token);
            break;

        case LXB_CSS_SYNTAX_TOKEN_LS_BRACKET:
            lxb_css_syntax_token_consume(parser->tkz);
            status = lxb_css_selectors_state_attribute(parser);
            break;

        case LXB_CSS_SYNTAX_TOKEN_COLON:
            lxb_css_syntax_token_consume(parser->tkz);
            lxb_css_parser_token_m(parser, token);

            if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
                status = lxb_css_selectors_state_pseudo_class(parser, token);
                break;
            }
            else if (token->type != LXB_CSS_SYNTAX_TOKEN_FUNCTION) {
                return lxb_css_parser_unexpected(parser);
            }

            status = lxb_css_selectors_state_pseudo_class_function(parser, token,
                                                                   NULL);
            break;

        case LXB_CSS_SYNTAX_TOKEN_WHITESPACE:
            lxb_css_syntax_token_consume(parser->tkz);
            lxb_css_parser_token_m(parser, token);
            goto again;

        default:
            return lxb_css_parser_unexpected(parser);
    }

    if (status == LXB_STATUS_ERROR_MEMORY_ALLOCATION) {
        return lxb_css_parser_memory_fail(parser);
    }

    return lxb_css_selectors_done(parser);
}

static lxb_status_t
lxb_css_selectors_state_hash(lxb_css_parser_t *parser,
                             lxb_css_syntax_token_t *token)
{
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors;

    selectors = parser->selectors;

    lxb_css_syntax_token_consume(parser->tkz);
    lxb_css_selectors_state_append(parser, selectors, selector);

    selector->type = LXB_CSS_SELECTOR_TYPE_ID;

    return lxb_css_syntax_token_string_dup(lxb_css_syntax_token_string(token),
                                      &selector->name, selectors->memory->mraw);
}

static lxb_status_t
lxb_css_selectors_state_class(lxb_css_parser_t *parser,
                              lxb_css_syntax_token_t *token)
{
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors;

    lxb_css_parser_token_status_m(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
        selectors = parser->selectors;

        lxb_css_syntax_token_consume(parser->tkz);
        lxb_css_selectors_state_append(parser, selectors, selector);

        selector->type = LXB_CSS_SELECTOR_TYPE_CLASS;

        return lxb_css_syntax_token_string_dup(lxb_css_syntax_token_string(token),
                                        &selector->name, selectors->memory->mraw);
    }

    return lxb_css_parser_unexpected_status(parser);
}

static lxb_status_t
lxb_css_selectors_state_element_ns(lxb_css_parser_t *parser,
                                   lxb_css_syntax_token_t *token)
{
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors;

    selectors = parser->selectors;

    lxb_css_selectors_state_append(parser, selectors, selector);

    selector->type = LXB_CSS_SELECTOR_TYPE_ANY;

    selector->name.data = lexbor_mraw_alloc(selectors->memory->mraw, 2);
    if (selector->name.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    selector->name.data[0] = '*';
    selector->name.data[1] = '\0';
    selector->name.length = 1;

    if (lxb_css_syntax_token_delim_char(token) == '*') {
        lxb_css_syntax_token_consume(parser->tkz);
        return lxb_css_selectors_state_ns(parser, selector);
    }

    return lxb_css_selectors_state_ns_ident(parser, selector);
}

static lxb_status_t
lxb_css_selectors_state_element(lxb_css_parser_t *parser,
                                lxb_css_syntax_token_t *token)
{
    lxb_status_t status;
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors;

    selectors = parser->selectors;

    lxb_css_selectors_state_append(parser, selectors, selector);

    selector->type = LXB_CSS_SELECTOR_TYPE_ELEMENT;

    lxb_css_selectors_state_string_dup_m(selectors, &selector->name);

    lxb_css_syntax_token_consume(parser->tkz);

    return lxb_css_selectors_state_ns(parser, selector);
}


static lxb_status_t
lxb_css_selectors_state_attribute(lxb_css_parser_t *parser)
{
    lxb_char_t modifier;
    lxb_status_t status;
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors;
    lxb_css_syntax_token_t *token;
    lxb_css_syntax_tokenizer_t *tkz;
    lxb_css_selector_attribute_t *attribute;

    selectors = parser->selectors;

    tkz = parser->tkz;

    lxb_css_selectors_state_append(parser, selectors, selector);
    lxb_css_parser_token_status_wo_ws_m(parser, token);

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_DELIM:
            if (lxb_css_syntax_token_delim_char(token) != '|') {
                goto failed;
            }

            lxb_css_syntax_token_consume(tkz);
            lxb_css_parser_token_status_m(parser, token);

            if (token->type != LXB_CSS_SYNTAX_TOKEN_IDENT) {
                goto failed;
            }

            selector->type = LXB_CSS_SELECTOR_TYPE_ATTRIBUTE;

            selector->ns.data = lexbor_mraw_alloc(selectors->memory->mraw, 2);
            if (selector->ns.data == NULL) {
                selectors->bracket = true;
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            selector->ns.data[0] = '*';
            selector->ns.data[1] = '\0';
            selector->ns.length = 1;

            lxb_css_selectors_state_string_dup_m(parser->selectors,
                                                 &selector->name);

            lxb_css_syntax_token_consume(tkz);
            lxb_css_parser_token_status_wo_ws_m(parser, token);
            break;

        case LXB_CSS_SYNTAX_TOKEN_IDENT:
            selector->type = LXB_CSS_SELECTOR_TYPE_ATTRIBUTE;

            lxb_css_selectors_state_string_dup_m(selectors, &selector->name);

            lxb_css_syntax_token_consume(tkz);
            lxb_css_parser_token_status_m(parser, token);

            if (token->type != LXB_CSS_SYNTAX_TOKEN_DELIM
                || lxb_css_syntax_token_delim_char(token) != '|')
            {
                lxb_css_syntax_token_consume(tkz);

                if (token->type == LXB_CSS_SYNTAX_TOKEN_WHITESPACE) {
                    lxb_css_parser_token_status_m(parser, token);
                }

                break;
            }

            lxb_css_syntax_token_consume(tkz);
            lxb_css_parser_token_status_m(parser, token);

            if (token->type != LXB_CSS_SYNTAX_TOKEN_IDENT) {
                goto failed;
            }

            selector->ns = selector->name;
            lexbor_str_clean_all(&selector->name);

            lxb_css_selectors_state_string_dup_m(selectors, &selector->name);

            lxb_css_syntax_token_consume(tkz);
            lxb_css_parser_token_status_wo_ws_m(parser, token);
            break;

        default:
            goto failed;
    }

    attribute = &selector->u.attribute;

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_RS_BRACKET:
            lxb_css_syntax_token_consume(tkz);
            return LXB_STATUS_OK;

        case LXB_CSS_SYNTAX_TOKEN_DELIM:
            switch (lxb_css_syntax_token_delim_char(token)) {
                case '~':
                    attribute->match = LXB_CSS_SELECTOR_MATCH_INCLUDE;
                    break;

                case '|':
                    attribute->match = LXB_CSS_SELECTOR_MATCH_DASH;
                    break;

                case '^':
                    attribute->match = LXB_CSS_SELECTOR_MATCH_PREFIX;
                    break;

                case '$':
                    attribute->match = LXB_CSS_SELECTOR_MATCH_SUFFIX;
                    break;

                case '*':
                    attribute->match = LXB_CSS_SELECTOR_MATCH_SUBSTRING;
                    break;

                case '=':
                    attribute->match = LXB_CSS_SELECTOR_MATCH_EQUAL;

                    lxb_css_syntax_token_consume(tkz);
                    lxb_css_parser_token_status_wo_ws_m(parser, token);
                    goto string_or_ident;

                default:
                    goto failed;
            }

            lxb_css_syntax_token_consume(tkz);
            lxb_css_parser_token_status_m(parser, token);

            if (token->type != LXB_CSS_SYNTAX_TOKEN_DELIM
                || lxb_css_syntax_token_delim_char(token) != '=')
            {
                goto failed;
            }

            lxb_css_syntax_token_consume(tkz);
            lxb_css_parser_token_status_wo_ws_m(parser, token);
            break;

        default:
            goto failed;
    }

string_or_ident:

    if (token->type != LXB_CSS_SYNTAX_TOKEN_STRING
        && token->type != LXB_CSS_SYNTAX_TOKEN_IDENT)
    {
        goto failed;
    }

    lxb_css_selectors_state_string_dup_m(selectors, &attribute->value);

    lxb_css_syntax_token_consume(tkz);
    lxb_css_parser_token_status_wo_ws_m(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_RS_BRACKET) {
        lxb_css_syntax_token_consume(tkz);
        return LXB_STATUS_OK;
    }

    if (token->type != LXB_CSS_SYNTAX_TOKEN_IDENT) {
        goto failed;
    }

    modifier = *lxb_css_syntax_token_string(token)->data;

    switch (modifier) {
        case 'i':
            attribute->modifier = LXB_CSS_SELECTOR_MODIFIER_I;
            break;

        case 's':
            attribute->modifier = LXB_CSS_SELECTOR_MODIFIER_S;
            break;

        default:
            goto failed;
    }

    lxb_css_syntax_token_consume(tkz);
    lxb_css_parser_token_status_wo_ws_m(parser, token);

    if (token->type != LXB_CSS_SYNTAX_TOKEN_RS_BRACKET) {
        goto failed;
    }

    lxb_css_syntax_token_consume(tkz);

    return LXB_STATUS_OK;

failed:

    selectors->bracket = true;

    return lxb_css_parser_unexpected_status(parser);
}

static lxb_status_t
lxb_css_selectors_state_ns(lxb_css_parser_t *parser,
                           lxb_css_selector_t *selector)
{
    lxb_css_syntax_token_t *token;

    lxb_css_parser_token_status_m(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_DELIM
        && lxb_css_syntax_token_delim_char(token) == '|')
    {
        lxb_css_syntax_token_consume(parser->tkz);
        return lxb_css_selectors_state_ns_ident(parser, selector);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_css_selectors_state_ns_ident(lxb_css_parser_t *parser,
                                 lxb_css_selector_t *selector)
{
    lxb_css_syntax_token_t *token;

    lxb_css_parser_token_status_m(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
        lxb_css_syntax_token_consume(parser->tkz);

        selector->type = LXB_CSS_SELECTOR_TYPE_ELEMENT;

        selector->ns = selector->name;
        lexbor_str_clean_all(&selector->name);

        return lxb_css_syntax_token_string_dup(lxb_css_syntax_token_string(token),
                                &selector->name, parser->selectors->memory->mraw);
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_DELIM
             && lxb_css_syntax_token_delim_char(token) == '*')
    {
        lxb_css_syntax_token_consume(parser->tkz);

        selector->type = LXB_CSS_SELECTOR_TYPE_ANY;

        selector->ns = selector->name;

        selector->name.data = lexbor_mraw_alloc(parser->selectors->memory->mraw,
                                                2);
        if (selector->name.data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        selector->name.data[0] = '*';
        selector->name.data[1] = '\0';
        selector->name.length = 1;

        return LXB_STATUS_OK;
    }

    return lxb_css_parser_unexpected_status(parser);
}

static lxb_status_t
lxb_css_selectors_state_pseudo_class(lxb_css_parser_t *parser,
                                     lxb_css_syntax_token_t *token)
{
    lxb_status_t status;
    lxb_css_log_message_t *msg;
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors;
    const lxb_css_selectors_pseudo_data_t *pseudo;

    selectors = parser->selectors;

    lxb_css_selectors_state_append(parser, selectors, selector);
    selector->type = LXB_CSS_SELECTOR_TYPE_PSEUDO_CLASS;

    lxb_css_selectors_state_string_dup_m(selectors, &selector->name);

    pseudo = lxb_css_selector_pseudo_class_by_name(selector->name.data,
                                                   selector->name.length);
    if (pseudo == NULL) {
        return lxb_css_parser_unexpected_status(parser);
    }

    switch (pseudo->id) {
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_CURRENT:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_DEFAULT:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_FOCUS_VISIBLE:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_FOCUS_WITHIN:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_FULLSCREEN:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_FUTURE:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_IN_RANGE:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_INDETERMINATE:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_INVALID:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_LOCAL_LINK:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_OUT_OF_RANGE:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_PAST:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_SCOPE:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_TARGET:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_TARGET_WITHIN:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_USER_INVALID:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_VALID:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_VISITED:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_WARNING:
            msg = lxb_css_log_not_supported(parser->log,
                                            lxb_css_selectors_module_name,
                                            (const char *) selector->name.data);
            if (msg == NULL) {
                return lxb_css_parser_memory_fail(parser);
            }

            return lxb_css_parser_unexpected_status(parser);

        default:
            break;
    }

    selector->u.pseudo.type = pseudo->id;
    selector->u.pseudo.data = NULL;

    lxb_css_syntax_token_consume(parser->tkz);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_css_selectors_state_pseudo_class_function(lxb_css_parser_t *parser,
                                              lxb_css_syntax_token_t *token,
                                              lxb_css_parser_state_f success)
{
    lxb_status_t status;
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors;
    lxb_css_log_message_t *msg;
    lxb_css_selector_list_t *list;
    lxb_css_parser_stack_t *entry;
    const lxb_css_selectors_pseudo_data_func_t *func;

    selectors = parser->selectors;

    lxb_css_selectors_state_append(parser, selectors, selector);
    selector->type = LXB_CSS_SELECTOR_TYPE_PSEUDO_CLASS_FUNCTION;

    lxb_css_selectors_state_string_dup_m(selectors, &selector->name);

    func = lxb_css_selector_pseudo_class_function_by_name(selector->name.data,
                                                          selector->name.length);
    if (func == NULL) {
        return lxb_css_parser_unexpected_status(parser);
    }

    switch (func->id) {
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_FUNCTION_DIR:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_FUNCTION_LANG:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_FUNCTION_NTH_COL:
        case LXB_CSS_SELECTOR_PSEUDO_CLASS_FUNCTION_NTH_LAST_COL:
            msg = lxb_css_log_not_supported(parser->log,
                                            lxb_css_selectors_module_name,
                                            (const char *) selector->name.data);
            if (msg == NULL) {
                return lxb_css_parser_memory_fail(parser);
            }

            return lxb_css_parser_unexpected_status(parser);

        default:
            break;
    }

    parser->state = func->state;

    selector->u.pseudo.type = func->id;
    selector->u.pseudo.data = NULL;

    list = selectors->list_last;

    if (success != NULL) {
        entry = lxb_css_parser_stack_push(parser, success, list, false);
        if (entry == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    entry = lxb_css_parser_stack_push(parser, func->success, list, true);
    if (entry == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    selectors->combinator = func->combinator;
    selectors->comb_default = func->combinator;
    selectors->parent = selector;

    lxb_css_syntax_token_consume(parser->tkz);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_css_selectors_state_pseudo_element(lxb_css_parser_t *parser,
                                       lxb_css_syntax_token_t *token)
{
    lxb_status_t status;
    lxb_css_log_message_t *msg;
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors;
    const lxb_css_selectors_pseudo_data_t *pseudo;

    selectors = parser->selectors;

    lxb_css_selectors_state_append(parser, selectors, selector);
    selector->type = LXB_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT;

    lxb_css_selectors_state_string_dup_m(selectors, &selector->name);

    pseudo = lxb_css_selector_pseudo_element_by_name(selector->name.data,
                                                     selector->name.length);
    if (pseudo == NULL) {
        return lxb_css_parser_unexpected_status(parser);
    }

    switch (pseudo->id) {
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_AFTER:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_BACKDROP:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_BEFORE:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_FIRST_LETTER:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_FIRST_LINE:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_GRAMMAR_ERROR:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_INACTIVE_SELECTION:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_MARKER:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_PLACEHOLDER:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_SELECTION:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_SPELLING_ERROR:
        case LXB_CSS_SELECTOR_PSEUDO_ELEMENT_TARGET_TEXT:
            msg = lxb_css_log_not_supported(parser->log,
                                            lxb_css_selectors_module_name,
                                            (const char *) selector->name.data);
            if (msg == NULL) {
                return lxb_css_parser_memory_fail(parser);
            }

            return lxb_css_parser_unexpected_status(parser);

        default:
            break;
    }

    selector->u.pseudo.type = pseudo->id;
    selector->u.pseudo.data = NULL;

    lxb_css_syntax_token_consume(parser->tkz);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_css_selectors_state_pseudo_element_function(lxb_css_parser_t *parser,
                                                lxb_css_syntax_token_t *token,
                                                lxb_css_parser_state_f success)
{
    lxb_status_t status;
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors;
    lxb_css_selector_list_t *list;
    lxb_css_parser_stack_t *entry;
    const lxb_css_selectors_pseudo_data_func_t *func;

    selectors = parser->selectors;

    lxb_css_selectors_state_append(parser, selectors, selector);
    selector->type = LXB_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT_FUNCTION;

    lxb_css_selectors_state_string_dup_m(selectors, &selector->name);

    func = lxb_css_selector_pseudo_element_function_by_name(selector->name.data,
                                                            selector->name.length);
    if (func == NULL) {
        return lxb_css_parser_unexpected_status(parser);
    }

    parser->state = func->state;

    selector->u.pseudo.type = func->id;
    selector->u.pseudo.data = NULL;

    list = selectors->list_last;

    entry = lxb_css_parser_stack_push(parser, success, list, false);
    if (entry == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    entry = lxb_css_parser_stack_push(parser, func->success, list, true);
    if (entry == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    selectors->combinator = func->combinator;
    selectors->comb_default = func->combinator;
    selectors->parent = selector;

    lxb_css_syntax_token_consume(parser->tkz);

    return LXB_STATUS_OK;
}

lxb_inline void
lxb_css_selectors_state_restore_combinator(lxb_css_selectors_t *selectors)
{
    lxb_css_selector_t *parent;
    lxb_css_selector_combinator_t comb_default;
    const lxb_css_selectors_pseudo_data_func_t *data_func;

    comb_default = LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT;

    if (selectors->parent != NULL) {
        parent = selectors->parent;

        if (parent->type == LXB_CSS_SELECTOR_TYPE_PSEUDO_CLASS_FUNCTION) {
            data_func = lxb_css_selector_pseudo_class_function_by_id(parent->u.pseudo.type);
        }
        else {
            data_func = lxb_css_selector_pseudo_element_function_by_id(parent->u.pseudo.type);
        }

        comb_default = data_func->combinator;
    }

    selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_CLOSE;
    selectors->comb_default = comb_default;
}

bool
lxb_css_selectors_state_success(lxb_css_parser_t *parser,
                                lxb_css_syntax_token_t *token, void *ctx)
{
    bool cy;
    lxb_css_selector_t *selector;
    lxb_css_selectors_t *selectors = parser->selectors;

    if (parser->status == LXB_STATUS_OK
        && token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS)
    {
        lxb_css_syntax_token_consume(parser->tkz);
        lxb_css_selectors_state_restore_parent(selectors, ctx);
        return lxb_css_selectors_done(parser);
    }

    /* Empty function. */
    if ((selectors->list == NULL || selectors->list->first == NULL)
        && token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS
        && selectors->deep == 0)
    {
        lxb_css_selector_list_destroy_chain(selectors->list);
        selectors->list = NULL;

        lxb_css_selectors_state_restore_parent(selectors, ctx);

        selector = selectors->list_last->last;

        cy = selector->type == LXB_CSS_SELECTOR_TYPE_PSEUDO_CLASS_FUNCTION;
        cy = lxb_css_selector_pseudo_function_can_empty(selector->u.pseudo.type,
                                                        cy);
        if (!cy) {
            (void) lxb_css_log_format(parser->log, LXB_CSS_LOG_ERROR,
                                      "%s. Pseudo function can't be empty: %S()",
                                      lxb_css_selectors_module_name,
                                      &selector->name);
            selectors->deep++;

            lxb_css_parser_stack_to_stop(parser);
            return false;
        }

        parser->status = LXB_STATUS_OK;

        lxb_css_syntax_token_consume(parser->tkz);
        return lxb_css_selectors_done(parser);
    }

    /* Set error. */
    selectors->deep++;

    parser->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;

    lxb_css_selector_list_destroy_chain(selectors->list);
    selectors->list = NULL;

    lxb_css_selectors_state_restore_parent(selectors, ctx);
    lxb_css_parser_stack_to_stop(parser);

    return false;
}

bool
lxb_css_selectors_state_forgiving(lxb_css_parser_t *parser,
                                  lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_forgiving_cb(parser, token, ctx,
                                                lxb_css_selectors_state_complex_list);
}

bool
lxb_css_selectors_state_forgiving_relative(lxb_css_parser_t *parser,
                                           lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_forgiving_cb(parser, token, ctx,
                                                lxb_css_selectors_state_relative_list);
}

static bool
lxb_css_selectors_state_forgiving_cb(lxb_css_parser_t *parser,
                                     lxb_css_syntax_token_t *token, void *ctx,
                                     lxb_css_parser_state_f state)
{
    bool cy;
    lxb_css_selector_t *selector;
    lxb_css_selector_list_t *list;
    const lxb_css_selectors_pseudo_data_func_t *func;
    lxb_css_selectors_t *selectors = parser->selectors;

    if (parser->status == LXB_STATUS_OK
        && token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS)
    {
        lxb_css_syntax_token_consume(parser->tkz);
        lxb_css_selectors_state_restore_parent(selectors, ctx);
        return lxb_css_selectors_done(parser);
    }

    /* Emply list. */
    if (token->type == LXB_CSS_SYNTAX_TOKEN_COMMA && selectors->deep == 0) {
        selectors->combinator = selectors->comb_default;

        lxb_css_syntax_token_consume(parser->tkz);

        /* Destroying a bad Selector List. */
        list = selectors->list_last;

        lxb_css_selector_list_selectors_remove(selectors, list);
        lxb_css_selector_list_destroy(list);

        /* Recovering the parser from the error. */
        lxb_css_parser_stack_up(parser);

        parser->status = LXB_STATUS_OK;
        parser->state = state;

        (void) lxb_css_log_format(parser->log, LXB_CSS_LOG_ERROR,
                                  "%s. Empty Selector List in pseudo function",
                                  lxb_css_selectors_module_name);
        return true;
    }

    /* End Of File. */
    if (token->type == LXB_CSS_SYNTAX_TOKEN__EOF) {
        selector = selectors->list_last->last;

        if (selector == NULL) {
            (void) lxb_css_log_format(parser->log, LXB_CSS_LOG_ERROR,
                                      "%s. End Of File in pseudo function",
                                      lxb_css_selectors_module_name);
        }
        else {
            func = NULL;

            if (selector->type == LXB_CSS_SELECTOR_TYPE_PSEUDO_CLASS_FUNCTION) {
                func =lxb_css_selector_pseudo_class_function_by_id(selector->u.pseudo.type);
            }
            else if (selector->type == LXB_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT_FUNCTION) {
                func =lxb_css_selector_pseudo_element_function_by_id(selector->u.pseudo.type);
            }

            if (func != NULL
                && func->success != lxb_css_selectors_state_forgiving
                && func->success != lxb_css_selectors_state_forgiving_relative)
            {
                (void) lxb_css_log_format(parser->log, LXB_CSS_LOG_ERROR,
                                          "%s. End Of File in pseudo function",
                                          lxb_css_selectors_module_name);
            }
        }

        if (parser->status == LXB_STATUS_OK) {
            lxb_css_selectors_state_restore_parent(selectors, ctx);
            lxb_css_parser_stack_to_stop(parser);

            return false;
        }

        goto failed;
    }

    if (lxb_css_syntax_token_error(parser, token, "Selectors") == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    /* Empty function. */
    if (token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        selectors->deep++;
        goto failed;
    }

    /* Destroying a bad Selector List. */
    list = selectors->list_last;

    lxb_css_selector_list_selectors_remove(selectors, list);
    lxb_css_selector_list_destroy(list);

    /* Close all open blocks. */
    token = lxb_css_selectors_close_parenthesis(parser, token);
    if (token == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    /* Find ',' or end of function ')' */
    token = lxb_css_parser_find_close(parser, token, NULL,
                                      LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS,
                                      LXB_CSS_SYNTAX_TOKEN_COMMA);
    if (token == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    /*
     * This is only necessary in order to print the correct message to the log.
     * Example: :has(1%, div)
     */
    if (token->type == LXB_CSS_SYNTAX_TOKEN_COMMA) {
        lxb_css_selectors_state_restore_combinator(selectors);

        selectors->combinator = selectors->comb_default;

        lxb_css_syntax_token_consume(parser->tkz);

        /* Recovering the parser from the error. */
        lxb_css_parser_stack_up(parser);

        parser->status = LXB_STATUS_OK;
        parser->state = state;

        return true;
    }

    if (token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        lxb_css_syntax_token_consume(parser->tkz);
        goto failed_done;
    }

    /* Recovering the parser from the error. */
    lxb_css_parser_stack_up(parser);

    parser->status = LXB_STATUS_OK;
    parser->state = state;

    return false;

failed:

    /* Destroying a bad Selector List. */
    list = selectors->list_last;

    lxb_css_selector_list_selectors_remove(selectors, list);
    lxb_css_selector_list_destroy(list);

    /* Close all open blocks. */
    token = lxb_css_selectors_close_parenthesis(parser, token);
    if (token == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

failed_done:

    cy = selectors->list == NULL;

    lxb_css_selectors_state_restore_parent(selectors, ctx);

    if (cy) {
        selector = selectors->list_last->last;

        cy = selector->type == LXB_CSS_SELECTOR_TYPE_PSEUDO_CLASS_FUNCTION;
        cy = lxb_css_selector_pseudo_function_can_empty(selector->u.pseudo.type,
                                                        cy);
        if (!cy) {
            (void) lxb_css_log_format(parser->log, LXB_CSS_LOG_ERROR,
                                      "%s. Pseudo function can't be empty: %S()",
                                      lxb_css_selectors_module_name,
                                      &selector->name);

            lxb_css_parser_stack_to_stop(parser);
            return true;
        }
    }

    parser->status = LXB_STATUS_OK;

    return lxb_css_selectors_done(parser);
}

static void
lxb_css_selectors_state_restore_parent(lxb_css_selectors_t *selectors,
                                       lxb_css_selector_list_t *last)
{
    if (selectors->list != NULL) {
        last->last->u.pseudo.data = selectors->list;
    }

    selectors->list_last = last;

    /* Get first Selector in chain. */
    while (last->prev != NULL) {
        last = last->prev;
    }

    selectors->list = last;
    selectors->parent = last->parent;

    lxb_css_selectors_state_restore_combinator(selectors);
}

static bool
lxb_css_selectors_state_list_end(lxb_css_parser_t *parser,
                                 lxb_css_syntax_token_t *token,
                                 lxb_css_parser_state_f state)
{
    if (token->type == LXB_CSS_SYNTAX_TOKEN_WHITESPACE) {
        lxb_css_syntax_token_consume(parser->tkz);
        lxb_css_parser_token_status_m(parser, token);
    }

    if (token->type != LXB_CSS_SYNTAX_TOKEN_COMMA) {
        return lxb_css_selectors_done(parser);
    }

    parser->selectors->combinator = parser->selectors->comb_default;

    lxb_css_syntax_token_consume(parser->tkz);
    lxb_css_parser_stack_up(parser);

    parser->state = state;

    return true;
}
