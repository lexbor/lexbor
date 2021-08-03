/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/css/parser.h"

lxb_css_parser_t *
lxb_css_parser_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_css_parser_t));
}

lxb_status_t
lxb_css_parser_init(lxb_css_parser_t *parser,
                    lxb_css_syntax_tokenizer_t *tkz, lexbor_mraw_t *mraw)
{
    lxb_status_t status;
    static const size_t lxb_stack_size = 1024;

    if (parser == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    /* Stack */
    parser->stack_begin = lexbor_malloc(sizeof(lxb_css_parser_stack_t)
                                        * lxb_stack_size);
    if (parser->stack_begin == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    parser->stack = parser->stack_begin;
    parser->stack_end = parser->stack_begin + lxb_stack_size;

    /* Syntax */
    parser->my_tkz = false;

    if (tkz == NULL) {
        tkz = lxb_css_syntax_tokenizer_create();
        status = lxb_css_syntax_tokenizer_init(tkz);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        parser->my_tkz = true;
    }

    /* Memmory for all structures. */
    parser->my_mraw = false;

    if (mraw == NULL) {
        mraw = lexbor_mraw_create();
        status = lexbor_mraw_init(mraw, 4096 * 4);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        parser->my_mraw = true;
    }

    parser->log = lxb_css_log_create();
    status = lxb_css_log_init(parser->log, NULL);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    parser->tkz = tkz;
    parser->mraw = mraw;
    parser->types_begin = NULL;
    parser->types_pos = NULL;
    parser->types_end = NULL;
    parser->stage = LXB_CSS_PARSER_CLEAN;

    return LXB_STATUS_OK;
}

void
lxb_css_parser_clean(lxb_css_parser_t *parser)
{
    lxb_css_syntax_tokenizer_clean(parser->tkz);
    lxb_css_log_clean(parser->log);

    if (parser->my_mraw) {
        lexbor_mraw_clean(parser->mraw);
    }

    parser->types_pos = parser->types_begin;
    parser->stack = parser->stack_begin;
    parser->status = LXB_STATUS_OK;
    parser->stage = LXB_CSS_PARSER_CLEAN;
}

lxb_css_parser_t *
lxb_css_parser_destroy(lxb_css_parser_t *parser, bool self_destroy)
{
    if (parser == NULL) {
        return NULL;
    }

    if (parser->my_tkz) {
        parser->tkz = lxb_css_syntax_tokenizer_destroy(parser->tkz);
    }

    parser->log = lxb_css_log_destroy(parser->log, true);

    if (parser->my_mraw) {
        if (lexbor_mraw_reference_count(parser->mraw) == 0) {
            parser->mraw = lexbor_mraw_destroy(parser->mraw, true);
        }
    }

    if (parser->stack_begin != NULL) {
        parser->stack_begin = lexbor_free(parser->stack_begin);
    }

    if (parser->types_begin != NULL) {
        parser->types_begin = lexbor_free(parser->types_begin);
    }

    if (self_destroy) {
        return lexbor_free(parser);
    }

    return parser;
}

lxb_css_parser_stack_t *
lxb_css_parser_stack_push(lxb_css_parser_t *parser,
                          lxb_css_parser_state_f state, void *ctx, bool stop)
{
    if (parser->stack >= parser->stack_end) {
        lxb_css_parser_stack_t *entry;

        size_t size = parser->stack_end - parser->stack_begin;
        size_t new_size = size + 1024;

        if (SIZE_MAX - size < 1024) {
            goto memory_error;
        }

        entry = lexbor_realloc(parser->stack_begin, new_size);
        if (entry == NULL) {
            goto memory_error;
        }

        parser->stack_begin = entry;
        parser->stack_end = entry + new_size;
        parser->stack = entry + size;
    }

    parser->stack->state = state;
    parser->stack->context = ctx;
    parser->stack->required_stop = stop;

    return parser->stack++;

memory_error:

    parser->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

    return NULL;
}

lxb_status_t
lxb_css_parser_run(lxb_css_parser_t *parser,
                   lxb_css_parser_state_f state, void *context)
{
    lxb_css_syntax_token_t *token;
    lxb_css_syntax_tokenizer_t *tkz;

    tkz = parser->tkz;

    parser->loop = true;
    parser->state = state;
    parser->context = context;

    do {
        token = lxb_css_syntax_token(tkz);
        if (token == NULL) {
            parser->status = tkz->status;
            return parser->status;
        }

        while (parser->state(parser, token, parser->context) == false) {};
    }
    while (parser->loop);

    return parser->status;
}

bool
lxb_css_parser_stop(lxb_css_parser_t *parser)
{
    parser->loop = false;
    return true;
}

bool
lxb_css_parser_fail(lxb_css_parser_t *parser, lxb_status_t status)
{
    parser->status = status;
    parser->loop = false;
    return true;
}

bool
lxb_css_parser_unexpected(lxb_css_parser_t *parser)
{
    (void) lxb_css_parser_unexpected_status(parser);
    return true;
}

lxb_status_t
lxb_css_parser_unexpected_status(lxb_css_parser_t *parser)
{
    parser->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;

    if (parser->selectors->list_last != NULL) {
        parser->selectors->list_last->invalid = true;
    }

    if (parser->stack > parser->stack_begin) {
        (void) lxb_css_parser_stack_to_stop(parser);
    }

    return LXB_STATUS_ERROR_UNEXPECTED_DATA;
}

bool
lxb_css_parser_unexpected_data(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token)
{
    static const char selectors[] = "Selectors";
    parser->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;

    if (parser->selectors->list_last != NULL) {
        parser->selectors->list_last->invalid = true;
    }

    if (lxb_css_syntax_token_error(parser, token, selectors) == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    if (parser->stack > parser->stack_begin) {
        (void) lxb_css_parser_stack_to_stop(parser);
    }

    return true;
}

lxb_status_t
lxb_css_parser_unexpected_data_status(lxb_css_parser_t *parser,
                                      const lxb_css_syntax_token_t *token)
{
    static const char selectors[] = "Selectors";
    parser->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;

    parser->selectors->list_last->invalid = true;

    if (lxb_css_syntax_token_error(parser, token, selectors) == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    if (parser->stack > parser->stack_begin) {
        (void) lxb_css_parser_stack_to_stop(parser);
    }

    return LXB_STATUS_ERROR_UNEXPECTED_DATA;
}

bool
lxb_css_parser_memory_fail(lxb_css_parser_t *parser)
{
    parser->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    parser->loop = false;
    return true;
}

lxb_inline lxb_status_t
lxb_css_parser_types_push(lxb_css_parser_t *parser,
                          lxb_css_syntax_token_type_t type)
{
    size_t length, new_length;
    lxb_css_syntax_token_type_t *tmp;

    if (parser->types_pos >= parser->types_end) {
        length = parser->types_end - parser->types_begin;

        if ((SIZE_MAX - length) < 1024) {
            return LXB_STATUS_ERROR_OVERFLOW;
        }

        new_length = length + 1024;

        tmp = lexbor_realloc(parser->types_begin,
                             new_length * sizeof(lxb_css_syntax_token_type_t));
        if (tmp == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        parser->types_begin = tmp;
        parser->types_end = tmp + new_length;
        parser->types_pos = parser->types_begin + length;
    }

    *parser->types_pos++ = type;

    return LXB_STATUS_OK;
}

lxb_inline bool
lxb_css_parser_types_done(lxb_css_syntax_token_t *token,
                          const lxb_char_t *opt_data, lxb_css_syntax_token_type_t type)
{
    if (opt_data == NULL) {
        return true;
    }

    switch (type) {
        case LXB_CSS_SYNTAX_TOKEN_DELIM:
            if (lxb_css_syntax_token_delim_char(token) == *opt_data) {
                return true;
            }

            return false;

        default:
            return true;
    }
}

lxb_css_syntax_token_t *
lxb_css_parser_find_close(lxb_css_parser_t *parser, lxb_css_syntax_token_t *token,
                          const lxb_char_t *opt_data, lxb_css_syntax_token_type_t type,
                          lxb_css_syntax_token_type_t stop)
{
    lxb_status_t status;
    lxb_css_syntax_token_type_t *pos;
    lxb_css_syntax_tokenizer_t *tkz = parser->tkz;

    parser->types_pos = parser->types_begin;
    pos = parser->types_pos;

    if (pos == NULL) {
        size_t length = 128;

        pos = lexbor_malloc(length * sizeof(lxb_css_syntax_token_type_t));
        if (pos == NULL) {
            status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            goto failed;
        }

        parser->types_begin = pos;
        parser->types_pos = pos;
        parser->types_end = pos + length;
    }

    status = lxb_css_parser_types_push(parser, type);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    do {
        switch (token->type) {
            case LXB_CSS_SYNTAX_TOKEN_LS_BRACKET:
                type = LXB_CSS_SYNTAX_TOKEN_RS_BRACKET;

                status = lxb_css_parser_types_push(parser, type);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                break;

            case LXB_CSS_SYNTAX_TOKEN_FUNCTION:
            case LXB_CSS_SYNTAX_TOKEN_L_PARENTHESIS:
                type = LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS;

                status = lxb_css_parser_types_push(parser, type);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                break;

            case LXB_CSS_SYNTAX_TOKEN_LC_BRACKET:
                type = LXB_CSS_SYNTAX_TOKEN_RC_BRACKET;

                status = lxb_css_parser_types_push(parser, type);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                break;

            case LXB_CSS_SYNTAX_TOKEN__EOF:
                return token;

            default:
                if (token->type == type) {
                    parser->types_pos--;

                    if (parser->types_pos == parser->types_begin) {
                        if (!lxb_css_parser_types_done(token, opt_data, type)) {
                            goto again;
                        }

                        return token;
                    }

                    type = *(parser->types_pos - 1);
                }
                else if (stop != LXB_CSS_SYNTAX_TOKEN_UNDEF
                    && stop == token->type)
                {
                    if (parser->types_pos - 1 == parser->types_begin) {
                        parser->types_pos--;
                        return token;
                    }
                }

                break;
        }

    again:

        lxb_css_syntax_token_consume(tkz);
        token = lxb_css_syntax_token(tkz);
    }
    while (token != NULL);

    status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

failed:

    parser->status = status;

    return NULL;
}

lxb_css_syntax_token_t *
lxb_css_parser_find_close_deep(lxb_css_parser_t *parser,
                               lxb_css_syntax_token_t *token,
                               lxb_css_syntax_token_type_t type, size_t *deep)
{
    size_t n = *deep;

    while (n != 0) {
        token = lxb_css_parser_find_close(parser, token, NULL, type,
                                          LXB_CSS_SYNTAX_TOKEN_UNDEF);
        if (token == NULL) {
            break;
        }

        if (token->type != LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
            break;
        }

        lxb_css_syntax_token_consume(parser->tkz);

        token = lxb_css_syntax_token(parser->tkz);
        if (token == NULL) {
            break;
        }

        n--;
    }

    *deep = n;

    return token;
}
