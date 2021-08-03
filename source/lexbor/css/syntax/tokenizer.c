/*
 * Copyright (C) 2018-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/css/syntax/tokenizer.h"
#include "lexbor/css/syntax/tokenizer/error.h"
#include "lexbor/css/syntax/state.h"
#include "lexbor/css/syntax/state_res.h"

#include "lexbor/core/array.h"


const lxb_char_t *lxb_css_syntax_tokenizer_eof = (const lxb_char_t *) "\x00";


static lxb_status_t
lxb_css_syntax_tokenizer_blank(lxb_css_syntax_tokenizer_t *tkz,
                               const lxb_char_t **data, const lxb_char_t **end,
                               void *ctx);

lxb_status_t
lxb_css_syntax_tokenizer_next_chunk(lxb_css_syntax_tokenizer_t *tkz,
                                    const lxb_char_t **data, const lxb_char_t **end);

lxb_status_t
lxb_css_syntax_state_tokens_realloc(lxb_css_syntax_tokenizer_t *tkz);


lxb_css_syntax_tokenizer_t *
lxb_css_syntax_tokenizer_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_css_syntax_tokenizer_t));
}

lxb_status_t
lxb_css_syntax_tokenizer_init(lxb_css_syntax_tokenizer_t *tkz)
{
    lxb_status_t status;

    static const unsigned tmp_size = 1024;
    static const unsigned tokens_length = 64;

    if (tkz == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    /* Init list for tokens. */
    tkz->tokens_begin = lexbor_calloc(tokens_length,
                                      sizeof(lxb_css_syntax_token_t));
    if (tkz->tokens_begin == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    tkz->tokens_end = tkz->tokens_begin + tokens_length;

    tkz->token = tkz->tokens_begin;
    tkz->last = tkz->token;
    tkz->prepared = NULL;

    tkz->mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(tkz->mraw, 4096);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Temp */
    tkz->start = lexbor_malloc(tmp_size);
    if (tkz->start == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    tkz->pos = tkz->start;
    tkz->end = tkz->start + tmp_size;

    /* Parse errors */
    tkz->parse_errors = lexbor_array_obj_create();
    status = lexbor_array_obj_init(tkz->parse_errors, 16,
                                   sizeof(lxb_css_syntax_tokenizer_error_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    tkz->eof = false;
    tkz->with_comment = false;
    tkz->status = LXB_STATUS_OK;
    tkz->opt = LXB_CSS_SYNTAX_TOKENIZER_OPT_UNDEF;
    tkz->chunk_cb = lxb_css_syntax_tokenizer_blank;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_css_syntax_tokenizer_clean(lxb_css_syntax_tokenizer_t *tkz)
{
    lexbor_mraw_clean(tkz->mraw);
    lexbor_array_obj_clean(tkz->parse_errors);

    tkz->token = tkz->tokens_begin;
    tkz->last = tkz->token;
    tkz->prepared = NULL;

    tkz->eof = false;
    tkz->status = LXB_STATUS_OK;
    tkz->in_begin = NULL;
    tkz->in_end = NULL;
    tkz->pos = tkz->start;

    return LXB_STATUS_OK;
}

lxb_css_syntax_tokenizer_t *
lxb_css_syntax_tokenizer_destroy(lxb_css_syntax_tokenizer_t *tkz)
{
    if (tkz == NULL) {
        return NULL;
    }

    if (tkz->tokens_begin != NULL) {
        tkz->tokens_begin = lexbor_free(tkz->tokens_begin);
        tkz->tokens_end = NULL;
    }

    tkz->mraw = lexbor_mraw_destroy(tkz->mraw, true);
    tkz->parse_errors = lexbor_array_obj_destroy(tkz->parse_errors, true);

    if (tkz->start != NULL) {
        tkz->start = lexbor_free(tkz->start);
    }

    return lexbor_free(tkz);
}

static lxb_status_t
lxb_css_syntax_tokenizer_blank(lxb_css_syntax_tokenizer_t *tkz,
                               const lxb_char_t **data, const lxb_char_t **end,
                               void *ctx)
{
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_css_syntax_tokenizer_chunk(lxb_css_syntax_tokenizer_t *tkz,
                               const lxb_char_t *data, size_t size)
{
    return tkz->status;
}

lxb_css_syntax_token_t *
lxb_css_syntax_tokenizer_token(lxb_css_syntax_tokenizer_t *tkz)
{
    lxb_status_t status;
    lxb_css_syntax_token_t *token;
    const lxb_char_t *begin, *end;

    begin = tkz->in_begin;
    end = tkz->in_end;

    if (tkz->prepared != NULL) {
        token = tkz->prepared++;

        if (tkz->prepared >= tkz->last) {
            tkz->prepared = NULL;
        }

        if (lxb_css_syntax_token_base(token)->end != NULL) {
            return token;
        }

        if (begin >= end) {
            status = lxb_css_syntax_tokenizer_next_chunk(tkz, &begin, &end);
            if (status != LXB_STATUS_OK) {
                return NULL;
            }

            if (begin >= end) {
                goto done;
            }
        }

        if (lxb_css_syntax_token_delim(token)->character == '-') {
            begin = lxb_css_syntax_state_minus_process(tkz, token, begin, end);
        }
        else {
            begin = lxb_css_syntax_state_plus_process(tkz, token, begin, end);
        }

        goto done;
    }

    if (tkz->last >= tkz->tokens_end) {
        tkz->status = lxb_css_syntax_state_tokens_realloc(tkz);
        if (tkz->status != LXB_STATUS_OK) {
            return NULL;
        }
    }

    token = tkz->last++;

again:

    if (begin >= end) {
        status = lxb_css_syntax_tokenizer_next_chunk(tkz, &begin, &end);
        if (status != LXB_STATUS_OK) {
            return NULL;
        }

        if (begin >= end) {
            token->type = LXB_CSS_SYNTAX_TOKEN__EOF;

            lxb_css_syntax_token_base(token)->begin = begin;
            lxb_css_syntax_token_base(token)->end = end;

            return token;
        }
    }

    token->cloned = false;

    begin = lxb_css_syntax_state_res_map[*begin](tkz, token, begin, end);

done:

    if (begin == NULL) {
        return NULL;
    }

    tkz->in_begin = begin;

    if (token->type == LXB_CSS_SYNTAX_TOKEN_COMMENT && tkz->with_comment) {
        end = tkz->in_end;
        goto again;
    }

    return token;
}

lxb_status_t
lxb_css_syntax_tokenizer_next_chunk(lxb_css_syntax_tokenizer_t *tkz,
                                    const lxb_char_t **data, const lxb_char_t **end)
{
    const lxb_char_t *begin;

    if (tkz->eof == false) {
        begin = *data;

        tkz->status = tkz->chunk_cb(tkz, data, end, tkz->chunk_ctx);
        if (tkz->status != LXB_STATUS_OK) {
            return tkz->status;
        }

        if (*data >= *end) {
            *data = begin;
            *end = begin;

            tkz->in_begin = begin;
            tkz->in_end = begin;

            tkz->eof = true;
        }
        else {
            tkz->in_begin = *data;
            tkz->in_end = *end;
        }
    }

    return LXB_STATUS_OK;
}

/*
 * No inline functions for ABI.
 */
lxb_status_t
lxb_css_syntax_tokenizer_status_noi(lxb_css_syntax_tokenizer_t *tkz)
{
    return lxb_css_syntax_tokenizer_status(tkz);
}
