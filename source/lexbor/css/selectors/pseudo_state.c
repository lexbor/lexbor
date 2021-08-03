/*
 * Copyright (C) 2020-2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/css/selectors/state.h"
#include "lexbor/css/selectors/pseudo_state.h"
#include "lexbor/css/selectors/selectors.h"


static bool
lxb_css_selectors_state_pseudo_anb(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_pseudo_of_begin(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx);

static bool
lxb_css_selectors_state_pseudo_of_end(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx);


lxb_inline bool
lxb_css_selectors_state_pseudo_anb_begin(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    (void) lxb_css_selectors_state_pseudo_anb(parser, token, ctx);
    (void) lxb_css_parser_stack_pop(parser);

    parser->selectors->list = NULL;
    parser->selectors->list_last = NULL;

    return true;
}


bool
lxb_css_selectors_state_pseudo_class_function__undef(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
}

bool
lxb_css_selectors_state_pseudo_class_function_current(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    parser->state = lxb_css_selectors_state_complex_list;

    parser->selectors->list = NULL;
    parser->selectors->list_last = NULL;

    return true;
}

bool
lxb_css_selectors_state_pseudo_class_function_dir(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
}

bool
lxb_css_selectors_state_pseudo_class_function_has(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    parser->state = lxb_css_selectors_state_relative_list;

    parser->selectors->list = NULL;
    parser->selectors->list_last = NULL;

    return true;
}

bool
lxb_css_selectors_state_pseudo_class_function_is(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    parser->state = lxb_css_selectors_state_complex_list;

    parser->selectors->list = NULL;
    parser->selectors->list_last = NULL;

    return true;
}

bool
lxb_css_selectors_state_pseudo_class_function_lang(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
}

bool
lxb_css_selectors_state_pseudo_class_function_not(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    parser->state = lxb_css_selectors_state_complex_list;

    parser->selectors->list = NULL;
    parser->selectors->list_last = NULL;

    return true;
}

bool
lxb_css_selectors_state_pseudo_class_function_nth_child(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_pseudo_of_begin(parser, token, ctx);
}

bool
lxb_css_selectors_state_pseudo_class_function_nth_col(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_pseudo_anb_begin(parser, token, ctx);
}

bool
lxb_css_selectors_state_pseudo_class_function_nth_last_child(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_pseudo_of_begin(parser, token, ctx);
}

bool
lxb_css_selectors_state_pseudo_class_function_nth_last_col(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_pseudo_anb_begin(parser, token, ctx);
}

bool
lxb_css_selectors_state_pseudo_class_function_nth_last_of_type(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_pseudo_anb_begin(parser, token, ctx);
}

bool
lxb_css_selectors_state_pseudo_class_function_nth_of_type(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_selectors_state_pseudo_anb_begin(parser, token, ctx);
}

bool
lxb_css_selectors_state_pseudo_class_function_where(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    parser->state = lxb_css_selectors_state_complex_list;

    parser->selectors->list = NULL;
    parser->selectors->list_last = NULL;

    return true;
}

bool
lxb_css_selectors_state_pseudo_element_function__undef(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    return true;
}

static bool
lxb_css_selectors_state_pseudo_anb(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_selectors_t *selectors;
    lxb_css_selector_list_t *list;
    lxb_css_selector_anb_of_t *anbof;

    selectors = parser->selectors;

    anbof = lexbor_mraw_alloc(selectors->memory->mraw,
                              sizeof(lxb_css_selector_anb_of_t));
    if (anbof == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    parser->status = lxb_css_syntax_anb_handler(parser, token, &anbof->anb);
    if (parser->status != LXB_STATUS_OK) {
        lexbor_mraw_free(selectors->memory->mraw, anbof);
        return true;
    }

    list = selectors->list_last;
    list->last->u.pseudo.data = anbof;

    anbof->of = NULL;

    return true;
}

static bool
lxb_css_selectors_state_pseudo_of_begin(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_selectors_t *selectors;
    lxb_css_parser_stack_t *entry;
    lxb_css_selector_list_t *list;
    lxb_css_syntax_token_ident_t *ident;

    static const lxb_char_t of[] = "of";

    selectors = parser->selectors;

    (void) lxb_css_selectors_state_pseudo_anb(parser, token, ctx);
    if (parser->status != LXB_STATUS_OK) {
        selectors->list = NULL;
        selectors->list_last = NULL;

        (void) lxb_css_parser_stack_to_stop(parser);

        return true;
    }

    list = selectors->list_last;

    selectors->list = NULL;
    selectors->list_last = NULL;

    lxb_css_parser_token_wo_ws_m(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
        ident = lxb_css_syntax_token_ident(token);

        if (ident->length == sizeof(of) - 1
            && lexbor_str_data_ncasecmp(ident->data, of, ident->length))
        {
            lxb_css_syntax_token_consume(parser->tkz);

            parser->state = lxb_css_selectors_state_complex_list;

            entry = lxb_css_parser_stack_push(parser,
                                              lxb_css_selectors_state_pseudo_of_end,
                                              list, true);
            if (entry == NULL) {
                lexbor_mraw_free(selectors->memory->mraw,
                                 list->last->u.pseudo.data);
                return lxb_css_parser_memory_fail(parser);
            }

            return true;
        }
    }

    (void) lxb_css_parser_stack_pop(parser);

    return true;
}

static bool
lxb_css_selectors_state_pseudo_of_end(lxb_css_parser_t *parser,
    lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_selector_anb_of_t *anbof;
    lxb_css_selector_list_t *list = ctx;

    anbof = list->last->u.pseudo.data;

    anbof->of = parser->selectors->list;

    parser->selectors->list = NULL;
    parser->selectors->list_last = NULL;

    if (parser->status != LXB_STATUS_OK) {
        (void) lxb_css_parser_stack_to_stop(parser);
        return true;
    }

    (void) lxb_css_parser_stack_pop(parser);

    return true;
}
