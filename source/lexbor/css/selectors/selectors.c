/*
 * Copyright (C) 2020-2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/core/print.h"
#include "lexbor/css/parser.h"
#include "lexbor/css/selectors/selectors.h"
#include "lexbor/css/selectors/state.h"


lxb_css_syntax_token_t *
lxb_css_selectors_close_parenthesis(lxb_css_parser_t *parser,
                                    lxb_css_syntax_token_t *token);

static lxb_css_selector_list_t *
lxb_css_selectors_parse_list(lxb_css_parser_t *parser, lxb_css_parser_state_f state,
                             const lxb_char_t *data, size_t length);

static bool
lxb_css_selectors_end(lxb_css_parser_t *parser,
                      lxb_css_syntax_token_t *token, void *ctx);


lxb_css_selectors_t *
lxb_css_selectors_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_css_selectors_t));
}

lxb_status_t
lxb_css_selectors_init(lxb_css_selectors_t *selectors, size_t prepare_count)
{
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    lexbor_dobject_t *objs;
    lxb_css_selectors_memory_t *mem = NULL;

    if (selectors == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    static const size_t size_mem = lexbor_max(sizeof(lxb_css_selector_t),
                                              sizeof(lxb_css_selector_list_t));

    if (prepare_count < 16) {
        prepare_count = 16;
    }

    objs = lexbor_dobject_create();
    status = lexbor_dobject_init(objs, (size_mem * prepare_count),
                                 size_mem);
    if (status != LXB_STATUS_OK) {
        mraw = NULL;
        goto failed;
    }

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 1034);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    mem = lexbor_malloc(sizeof(lxb_css_selectors_memory_t));
    if (mem == NULL) {
        status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        goto failed;
    }

    mem->mraw = mraw;
    mem->objs = objs;
    selectors->memory = mem;

    selectors->list = NULL;
    selectors->list_last = NULL;
    selectors->parent = NULL;
    selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT;
    selectors->comb_default = LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT;
    selectors->deep = 0;
    selectors->bracket = false;

    return LXB_STATUS_OK;

failed:

    lexbor_dobject_destroy(objs, true);
    lexbor_mraw_destroy(mraw, true);

    selectors->memory = NULL;

    return status;
}

void
lxb_css_selectors_clean(lxb_css_selectors_t *selectors)
{
    if (selectors != NULL) {
        selectors->list = NULL;
        selectors->list_last = NULL;
        selectors->parent = NULL;
        selectors->combinator = LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT;
        selectors->comb_default = LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT;
        selectors->deep = 0;
        selectors->bracket = false;
    }
}

void
lxb_css_selectors_erase(lxb_css_selectors_t *selectors)
{
    if (selectors != NULL) {
        lexbor_dobject_clean(selectors->memory->objs);
        lexbor_mraw_clean(selectors->memory->mraw);

        lxb_css_selectors_clean(selectors);
    }
}

lxb_css_selectors_t *
lxb_css_selectors_destroy(lxb_css_selectors_t *selectors,
                          bool with_memory, bool self_destroy)
{
    if (selectors == NULL) {
        return NULL;
    }

    if (with_memory) {
        (void) lexbor_dobject_destroy(selectors->memory->objs, true);
        (void) lexbor_mraw_destroy(selectors->memory->mraw, true);

        selectors->memory = lexbor_free(selectors->memory);
    }

    if (self_destroy) {
        return lexbor_free(selectors);
    }

    return selectors;
}

void
lxb_css_selectors_parser_destroy_list(lxb_css_parser_t *parser)
{
    lxb_css_selector_list_destroy_chain(parser->selectors->list);

    parser->selectors->list = NULL;
    parser->selectors->list_last = NULL;
}

lxb_css_selector_list_t *
lxb_css_selectors_parse(lxb_css_parser_t *parser,
                        const lxb_char_t *data, size_t length)
{
    return lxb_css_selectors_parse_complex_list(parser, data, length);
}

lxb_css_selector_list_t *
lxb_css_selectors_parse_complex_list(lxb_css_parser_t *parser,
                                     const lxb_char_t *data, size_t length)
{
    return lxb_css_selectors_parse_list(parser, lxb_css_selectors_state_complex_list,
                                        data, length);
}

lxb_css_selector_list_t *
lxb_css_selectors_parse_compound_list(lxb_css_parser_t *parser,
                                      const lxb_char_t *data, size_t length)
{
    return lxb_css_selectors_parse_list(parser, lxb_css_selectors_state_compound_list,
                                        data, length);
}

lxb_css_selector_list_t *
lxb_css_selectors_parse_simple_list(lxb_css_parser_t *parser,
                                    const lxb_char_t *data, size_t length)
{
    return lxb_css_selectors_parse_list(parser, lxb_css_selectors_state_simple_list,
                                        data, length);
}

lxb_css_selector_list_t *
lxb_css_selectors_parse_relative_list(lxb_css_parser_t *parser,
                                      const lxb_char_t *data, size_t length)
{
    return lxb_css_selectors_parse_list(parser, lxb_css_selectors_state_relative_list,
                                        data, length);
}

static lxb_css_selector_list_t *
lxb_css_selectors_parse_list(lxb_css_parser_t *parser, lxb_css_parser_state_f state,
                             const lxb_char_t *data, size_t length)
{
    lxb_status_t status;
    lxb_css_selector_list_t *list;
    lxb_css_parser_stack_t *stack;
    lxb_css_selectors_t selectors;

    if (parser->stage != LXB_CSS_PARSER_CLEAN) {
        if (parser->stage == LXB_CSS_PARSER_RUN) {
            parser->status = LXB_STATUS_ERROR_WRONG_ARGS;
            return NULL;
        }

        lxb_css_parser_clean(parser);
    }

    lxb_css_parser_buffer_set(parser, data, length);

    stack = lxb_css_parser_stack_push(parser, lxb_css_selectors_end, NULL, true);
    if (stack == NULL) {
        return NULL;
    }

    if (parser->selectors == NULL) {
        parser->selectors = &selectors;

        parser->status = lxb_css_selectors_init(parser->selectors, 32);
        if (parser->status != LXB_STATUS_OK) {
            parser->selectors = lxb_css_selectors_destroy(parser->selectors,
                                                          true, false);
            return NULL;
        }
    }
    else {
        lxb_css_selectors_clean(parser->selectors);
    }

    parser->stage = LXB_CSS_PARSER_RUN;

    status = lxb_css_parser_run(parser, state, NULL);
    if (status != LXB_STATUS_OK) {
        lxb_css_selectors_parser_destroy_list(parser);
    }

    list = parser->selectors->list;

    if (parser->selectors == &selectors) {
        (void) lxb_css_selectors_destroy(parser->selectors,
                                         list == NULL, false);
        parser->selectors = NULL;
    }

    parser->stage = LXB_CSS_PARSER_END;

    return list;
}

lxb_css_selector_list_t *
lxb_css_selectors_parse_complex(lxb_css_parser_t *parser,
                                const lxb_char_t *data, size_t length)
{
    return lxb_css_selectors_parse_list(parser, lxb_css_selectors_state_complex,
                                        data, length);
}

lxb_css_selector_list_t *
lxb_css_selectors_parse_relative(lxb_css_parser_t *parser,
                                 const lxb_char_t *data, size_t length)
{
    return lxb_css_selectors_parse_list(parser, lxb_css_selectors_state_relative,
                                        data, length);
}

lxb_css_selector_list_t *
lxb_css_selectors_parse_compound(lxb_css_parser_t *parser,
                                 const lxb_char_t *data, size_t length)
{
    return lxb_css_selectors_parse_list(parser, lxb_css_selectors_state_compound,
                                        data, length);
}

lxb_css_selector_list_t *
lxb_css_selectors_parse_simple(lxb_css_parser_t *parser,
                               const lxb_char_t *data, size_t length)
{
    return lxb_css_selectors_parse_list(parser, lxb_css_selectors_state_simple,
                                        data, length);
}

static bool
lxb_css_selectors_end(lxb_css_parser_t *parser,
                      lxb_css_syntax_token_t *token, void *ctx)
{
    if (parser->status != LXB_STATUS_OK) {
        if (lxb_css_syntax_token_error(parser, token, "Selectors") == NULL) {
            return lxb_css_parser_memory_fail(parser);
        }

        token = lxb_css_selectors_close_parenthesis(parser, token);
        if (token == NULL) {
            return lxb_css_parser_memory_fail(parser);
        }
    }
    else if (token->type != LXB_CSS_SYNTAX_TOKEN__EOF) {
        if (lxb_css_syntax_token_error(parser, token, "Selectors") == NULL) {
            return lxb_css_parser_memory_fail(parser);
        }

        parser->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
    }

    return lxb_css_parser_stop(parser);
}

lxb_css_syntax_token_t *
lxb_css_selectors_close_parenthesis(lxb_css_parser_t *parser,
                                    lxb_css_syntax_token_t *token)
{
    lxb_css_selectors_t *selectors = parser->selectors;

    if (token->type == LXB_CSS_SYNTAX_TOKEN_FUNCTION) {
        parser->selectors->deep++;

        lxb_css_syntax_token_consume(parser->tkz);
        token = lxb_css_syntax_token(parser->tkz);
    }

    if (selectors->bracket) {
        token = lxb_css_parser_find_close(parser, token, NULL,
                                          LXB_CSS_SYNTAX_TOKEN_RS_BRACKET,
                                          LXB_CSS_SYNTAX_TOKEN_UNDEF);
        if (token != NULL
            && token->type == LXB_CSS_SYNTAX_TOKEN_RS_BRACKET)
        {
            selectors->bracket = false;

            lxb_css_syntax_token_consume(parser->tkz);
            token = lxb_css_syntax_token(parser->tkz);
        }
    }

    if (token == NULL) {
        return NULL;
    }

    return lxb_css_parser_find_close_deep(parser, token,
                                          LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS,
                                          &selectors->deep);
}
