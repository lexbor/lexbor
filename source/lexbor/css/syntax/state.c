/*
 * Copyright (C) 2018-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <string.h>
#include <float.h>

#include "lexbor/core/utils.h"
#include "lexbor/core/strtod.h"

#include "lexbor/css/syntax/state.h"
#include "lexbor/css/syntax/tokenizer/error.h"

#define LXB_CSS_SYNTAX_RES_NAME_MAP
#include "lexbor/css/syntax/res.h"

#define LEXBOR_STR_RES_MAP_HEX
#define LEXBOR_STR_RES_ANSI_REPLACEMENT_CHARACTER
#include "lexbor/core/str_res.h"


#define LXB_CSS_SYNTAX_NEXT_CHUNK(_tkz, _status, _data, _end)                  \
    do {                                                                       \
        _status = lxb_css_syntax_tokenizer_next_chunk(_tkz, &_data, &_end);    \
        if (_status != LXB_STATUS_OK) {                                        \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)


#define LXB_CSS_SYNTAX_STR_APPEND_LEN(_tkz, _status, _begin, _length)          \
    do {                                                                       \
        _status = lxb_css_syntax_string_append(_tkz, _begin, _length);         \
        if (_status != LXB_STATUS_OK) {                                        \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)

#define LXB_CSS_SYNTAX_STR_APPEND(_tkz, _status, _begin, _end)                 \
    LXB_CSS_SYNTAX_STR_APPEND_LEN(_tkz, _status, _begin, (_end - _begin))


lxb_status_t
lxb_css_syntax_tokenizer_next_chunk(lxb_css_syntax_tokenizer_t *tkz,
                                    const lxb_char_t **data, const lxb_char_t **end);

lxb_status_t
lxb_css_syntax_state_tokens_realloc(lxb_css_syntax_tokenizer_t *tkz);


static const lxb_char_t *
lxb_css_syntax_state_consume_numeric(lxb_css_syntax_tokenizer_t *tkz,
                                     lxb_css_syntax_token_t *token,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end);

static const lxb_char_t *
lxb_css_syntax_state_decimal(lxb_css_syntax_tokenizer_t *tkz,
                             lxb_css_syntax_token_t *token,
                             lxb_char_t *buf_start, lxb_char_t *buf_end,
                             const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
lxb_css_syntax_state_consume_numeric_name_start(lxb_css_syntax_tokenizer_t *tkz,
                                                lxb_css_syntax_token_t *token,
                                                const lxb_char_t *data,
                                                const lxb_char_t *end);

static const lxb_char_t *
lxb_css_syntax_state_consume_ident(lxb_css_syntax_tokenizer_t *tkz,
                                   lxb_css_syntax_token_t *token,
                                   const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
lxb_css_syntax_state_url(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                         const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
lxb_css_syntax_state_bad_url(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                             const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
lxb_css_syntax_state_escaped(lxb_css_syntax_tokenizer_t *tkz,
                             const lxb_char_t *data, const lxb_char_t **end);

static const lxb_char_t *
lxb_css_syntax_state_escaped_string(lxb_css_syntax_tokenizer_t *tkz,
                                    const lxb_char_t *data, const lxb_char_t **end);


lxb_inline lxb_status_t
lxb_css_syntax_string_realloc(lxb_css_syntax_tokenizer_t *tkz, size_t upto)
{
    size_t len = tkz->pos - tkz->start;
    size_t size = (tkz->end - tkz->start) + upto;

    lxb_char_t *tmp = lexbor_realloc(tkz->start, size);
    if (tmp == NULL) {
        tkz->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        return tkz->status;
    }

    tkz->start = tmp;
    tkz->pos = tmp + len;
    tkz->end = tmp + size;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_css_syntax_string_append(lxb_css_syntax_tokenizer_t *tkz,
                             const lxb_char_t *data, size_t length)
{
    if ((tkz->end - tkz->pos) <= length) {
        if (lxb_css_syntax_string_realloc(tkz, length + 1024) != LXB_STATUS_OK) {
            return tkz->status;
        }
    }

    memcpy(tkz->pos, data, length);

    tkz->pos += length;

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_css_syntax_state_string_term(lxb_css_syntax_tokenizer_t *tkz)
{
    if (tkz->pos >= tkz->end) {
        if (lxb_css_syntax_string_realloc(tkz, 1024) != LXB_STATUS_OK) {
            return tkz->status;
        }
    }

    *tkz->pos = 0x00;

    return LXB_STATUS_OK;
}


lxb_inline const lxb_char_t *
lxb_css_syntax_state_string_set(lxb_css_syntax_tokenizer_t *tkz,
                                lxb_css_syntax_token_t *token,
                                const lxb_char_t *data)
{
    if(lxb_css_syntax_state_string_term(tkz) != LXB_STATUS_OK) {
        return NULL;
    }

    lxb_css_syntax_token_string(token)->data = tkz->start;
    lxb_css_syntax_token_string(token)->length = tkz->pos - tkz->start;

    tkz->pos = tkz->start;

    return data;
}

lxb_inline const lxb_char_t *
lxb_css_syntax_state_dimension_set(lxb_css_syntax_tokenizer_t *tkz,
                                   lxb_css_syntax_token_t *token,
                                   const lxb_char_t *data)
{
    if(lxb_css_syntax_state_string_term(tkz) != LXB_STATUS_OK) {
        return NULL;
    }

    lxb_css_syntax_token_dimension_string(token)->data = tkz->start;
    lxb_css_syntax_token_dimension_string(token)->length = tkz->pos - tkz->start;

    tkz->pos = tkz->start;

    return data;
}

static lxb_css_syntax_token_t *
lxb_css_syntax_tokenizer_token_append(lxb_css_syntax_tokenizer_t *tkz)
{
    if (tkz->prepared == NULL) {
        if (tkz->last >= tkz->tokens_end) {
            tkz->status = lxb_css_syntax_state_tokens_realloc(tkz);
            if (tkz->status != LXB_STATUS_OK) {
                return NULL;
            }
        }

        tkz->prepared = tkz->last;
        tkz->prepared->cloned = false;

        return tkz->last++;
    }

    lxb_css_syntax_token_t *first;
    size_t length = tkz->last - tkz->prepared;

    if ((tkz->last + length) >= tkz->tokens_end) {
        tkz->status = lxb_css_syntax_state_tokens_realloc(tkz);
        if (tkz->status != LXB_STATUS_OK) {
            return NULL;
        }
    }

    first = tkz->prepared;

    memmove(&first[1], first, length * sizeof(lxb_css_syntax_token_t));

    tkz->last++;
    first->cloned = false;

    return first;
}

lxb_status_t
lxb_css_syntax_state_tokens_realloc(lxb_css_syntax_tokenizer_t *tkz)
{
    lxb_css_syntax_token_t *tokens;

    static const unsigned length = 64;
    size_t new_length = (tkz->tokens_end - tkz->tokens_begin) + length;

    tokens = lexbor_calloc(new_length, sizeof(lxb_css_syntax_token_t));
    if (tokens == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    memcpy(tokens, tkz->token, (tkz->last - tkz->token)
                                * sizeof(lxb_css_syntax_token_t));

    if (tkz->prepared != NULL) {
        tkz->prepared = tokens + (tkz->prepared - tkz->token);
    }

    tkz->token = tokens;
    tkz->last = tokens + (tkz->last - tkz->tokens_begin);

    lexbor_free(tkz->tokens_begin);

    tkz->tokens_begin = tokens;
    tkz->tokens_end = tokens + new_length;

    return LXB_STATUS_OK;
}

/*
 * Delim
 */
lxb_inline lxb_css_syntax_token_t *
lxb_css_syntax_list_append_delim(lxb_css_syntax_tokenizer_t *tkz,
                                 const lxb_char_t *data,
                                 const lxb_char_t *end, lxb_char_t ch)
{
    lxb_css_syntax_token_t *delim;

    delim = lxb_css_syntax_tokenizer_token_append(tkz);
    if (delim == NULL) {
        return NULL;
    }

    delim->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

    lxb_css_syntax_token_base(delim)->begin = data;
    lxb_css_syntax_token_base(delim)->end = end;
    lxb_css_syntax_token_delim(delim)->character = ch;

    return delim;
}

lxb_inline void
lxb_css_syntax_state_delim_set(lxb_css_syntax_token_t *token, const lxb_char_t *begin,
                           const lxb_char_t *end, lxb_char_t ch)
{
    lxb_css_syntax_token_delim(token)->character = ch;
    lxb_css_syntax_token_base(token)->begin = begin;
    lxb_css_syntax_token_base(token)->end = end;

    token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;
}

const lxb_char_t *
lxb_css_syntax_state_delim(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                           const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_css_syntax_state_delim_set(token, data, data + 1, *data);

    return data + 1;
}

/*
 * Comment
 */
const lxb_char_t *
lxb_css_syntax_state_comment(lxb_css_syntax_tokenizer_t *tkz,
                             lxb_css_syntax_token_t *token,
                             const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_status_t status;
    const lxb_char_t *begin;

    lxb_css_syntax_token_base(token)->begin = data;

    /* Skip forward slash (/) */
    data++;

    if (data >= end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            goto delim;
        }
    }

    /* U+002A ASTERISK (*) */
    if (*data != 0x2A) {
        goto delim;
    }

    begin = data + 1;

    do {
        data++;

        if (data >= end) {
            if (begin < data) {
                LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
            }

            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                goto error;
            }

            begin = data;
        }

        switch (*data) {
            case 0x00:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                              lexbor_str_res_ansi_replacement_character,
                                              sizeof(lexbor_str_res_ansi_replacement_character) - 1);
                begin = data + 1;
                break;

            case 0x0D:
                data++;

                LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);

                tkz->pos[-1] = '\n';

                if (data >= end) {
                    LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
                    if (data >= end) {
                        goto error;
                    }
                }

                if (*data != 0x0A) {
                    data--;
                }

                begin = data + 1;
                break;

            case 0x0C:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                              (lxb_char_t *) "\n", 1);
                begin = data + 1;
                break;

            /* U+002A ASTERISK (*) */
            case 0x2A:
                data++;

                if (data >= end) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);

                    LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
                    if (data >= end) {
                        goto error;
                    }

                    if (*data == 0x2F) {
                        tkz->pos--;
                        *tkz->pos = 0x00;

                        data++;

                        goto done;
                    }

                    begin = data;
                }

                /* U+002F Forward slash (/) */
                if (*data == 0x2F) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, (data - 1));

                    data++;

                    goto done;
                }

                data--;
                break;
        }
    }
    while (true);

done:

    token->type = LXB_CSS_SYNTAX_TOKEN_COMMENT;

    lxb_css_syntax_token_base(token)->end = data;
    return lxb_css_syntax_state_string_set(tkz, token, data);

delim:

    token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

    lxb_css_syntax_token_base(token)->end = lxb_css_syntax_token_base(token)->begin + 1;
    lxb_css_syntax_token_delim(token)->character = '/';

    return data;

error:

    token->type = LXB_CSS_SYNTAX_TOKEN_COMMENT;

    lxb_css_syntax_token_base(token)->end = data;

    lxb_css_syntax_tokenizer_error_add(tkz->parse_errors, NULL,
                                       LXB_CSS_SYNTAX_TOKENIZER_ERROR_EOINCO);

    return lxb_css_syntax_state_string_set(tkz, token, data);
}

/*
 * Whitespace
 */
const lxb_char_t *
lxb_css_syntax_state_whitespace(lxb_css_syntax_tokenizer_t *tkz,
                                lxb_css_syntax_token_t *token,
                                const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_status_t status;
    const lxb_char_t *begin;

    token->type = LXB_CSS_SYNTAX_TOKEN_WHITESPACE;

    lxb_css_syntax_token_base(token)->begin = data;

    begin = data;

    do {
        switch (*data) {
            case 0x0D:
                data++;

                LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);

                tkz->pos[-1] = '\n';

                if (data >= end) {
                    LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
                    if (data >= end) {
                        goto done;
                    }
                }

                if (*data != 0x0A) {
                    data--;
                }

                begin = data + 1;
                break;

            case 0x0C:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                              (const lxb_char_t *) "\n", 1);
                begin = data + 1;
                break;

            case 0x09:
            case 0x20:
            case 0x0A:
                break;

            default:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                lxb_css_syntax_token_base(token)->end = data;

                return lxb_css_syntax_state_string_set(tkz, token, data);
        }

        data++;

        if (data >= end) {
            if (begin < data) {
                LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
            }

            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                break;
            }

            begin = data;
        }
    }
    while (true);

done:

    lxb_css_syntax_token_base(token)->end = data;

    return lxb_css_syntax_state_string_set(tkz, token, data);
}

/*
 * String token for U+0022 Quotation Mark (") and U+0027 Apostrophe (')
 */
const lxb_char_t *
lxb_css_syntax_state_string(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                            const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_char_t mark;
    lxb_status_t status;
    const lxb_char_t *begin;

    lxb_css_syntax_token_base(token)->begin = data;

    mark = *data++;
    begin = data;

    for (;; data++) {
        if (data >= end) {
            if (begin < data) {
                LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
            }

            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                goto error;
            }

            begin = data;
        }

        switch (*data) {
            case 0x00:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                              lexbor_str_res_ansi_replacement_character,
                                              sizeof(lexbor_str_res_ansi_replacement_character) - 1);
                begin = data + 1;
                break;

            /*
             * U+000A LINE FEED
             * U+000D CARRIAGE RETURN
             * U+000C FORM FEED
             */
            case 0x0A:
            case 0x0D:
            case 0x0C:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                lxb_css_syntax_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_CSS_SYNTAX_TOKENIZER_ERROR_NEINST);

                token->type = LXB_CSS_SYNTAX_TOKEN_BAD_STRING;

                lxb_css_syntax_token_base(token)->end = data;

                return lxb_css_syntax_state_string_set(tkz, token, data);

            /* U+005C REVERSE SOLIDUS (\) */
            case 0x5C:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                data++;

                if (data >= end) {
                    LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
                    if (data >= end) {
                        LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                                  (const lxb_char_t *) "\\", 1);
                        goto error;
                    }
                }

                data = lxb_css_syntax_state_escaped_string(tkz, data, &end);
                if (data == NULL) {
                    return NULL;
                }

                begin = data;

                data--;
                break;

            default:
                /* '"' or '\'' */
                if (*data == mark) {
                    if (begin < data) {
                        LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                    }

                    token->type = LXB_CSS_SYNTAX_TOKEN_STRING;

                    lxb_css_syntax_token_base(token)->end = ++data;

                    return lxb_css_syntax_state_string_set(tkz, token, data);
                }

                break;
        }
    }

    return data;

error:

    lxb_css_syntax_tokenizer_error_add(tkz->parse_errors, NULL,
                                       LXB_CSS_SYNTAX_TOKENIZER_ERROR_EOINST);

    token->type = LXB_CSS_SYNTAX_TOKEN_STRING;

    lxb_css_syntax_token_base(token)->end = data;

    return lxb_css_syntax_state_string_set(tkz, token, data);
}

/*
 * U+0023 NUMBER SIGN (#)
 */
const lxb_char_t *
lxb_css_syntax_state_hash(lxb_css_syntax_tokenizer_t *tkz,
                          lxb_css_syntax_token_t *token, const lxb_char_t *data,
                          const lxb_char_t *end)
{
    lxb_char_t ch;
    lxb_status_t status;
    const lxb_char_t *begin;
    lxb_css_syntax_token_t *delim;

    lxb_css_syntax_token_base(token)->begin = data++;

    if (data >= end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            goto delim;
        }
    }

    if (lxb_css_syntax_res_name_map[*data] == 0x00) {
        if (*data == 0x00) {
            goto hash;
        }

        /* U+005C REVERSE SOLIDUS (\) */
        if (*data != 0x5C) {
            goto delim;
        }

        begin = data++;

        if (data >= end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                goto push_delim;
            }
        }

        ch = *data;

        if (ch == 0x0A || ch == 0x0C || ch == 0x0D) {
            goto push_delim;
        }

        data = lxb_css_syntax_state_escaped(tkz, data, &end);
        if (data == NULL) {
            return NULL;
        }
    }

hash:

    token->type = LXB_CSS_SYNTAX_TOKEN_HASH;

    return lxb_css_syntax_state_consume_ident(tkz, token, data, end);

push_delim:

    delim = lxb_css_syntax_list_append_delim(tkz, begin, begin + 1, '\\');
    if (delim == NULL) {
        return NULL;
    }

delim:

    token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

    lxb_css_syntax_token_base(token)->end = lxb_css_syntax_token_base(token)->begin + 1;
    lxb_css_syntax_token_delim(token)->character = '#';

    return data;
}

/*
 * U+0028 LEFT PARENTHESIS (()
 */
const lxb_char_t *
lxb_css_syntax_state_lparenthesis(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                                  const lxb_char_t *data, const lxb_char_t *end)
{
    token->type = LXB_CSS_SYNTAX_TOKEN_L_PARENTHESIS;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_base(token)->end = ++data;

    return data;
}

/*
 * U+0029 RIGHT PARENTHESIS ())
 */
const lxb_char_t *
lxb_css_syntax_state_rparenthesis(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                                  const lxb_char_t *data, const lxb_char_t *end)
{
    token->type = LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_base(token)->end = ++data;

    return data;
}

/*
 * U+002B PLUS SIGN (+)
 */
const lxb_char_t *
lxb_css_syntax_state_plus(lxb_css_syntax_tokenizer_t *tkz,
                          lxb_css_syntax_token_t *token,
                          const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_status_t status;

    lxb_css_syntax_token_base(token)->begin = data++;

    if (data >= end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

            lxb_css_syntax_token_base(token)->end = data;
            lxb_css_syntax_token_delim(token)->character = '+';

            return data;
        }
    }

    return lxb_css_syntax_state_plus_process(tkz, token, data, end);
}

const lxb_char_t *
lxb_css_syntax_state_plus_process(lxb_css_syntax_tokenizer_t *tkz,
                                  lxb_css_syntax_token_t *token,
                                  const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_status_t status;
    const lxb_char_t *begin;
    lxb_css_syntax_token_t *delim;

    /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
    if (*data >= 0x30 && *data <= 0x39) {
        lxb_css_syntax_token_number(token)->have_sign = true;
        return lxb_css_syntax_state_consume_numeric(tkz, token, data, end);
    }

    /* U+002E FULL STOP (.) */
    if (*data == 0x2E) {
        begin = data++;

        if (data == end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);

            if (data >= end || *data < 0x30 || *data > 0x39) {
                goto push_delim;
            }

            lxb_css_syntax_token_number(token)->have_sign = true;

            return lxb_css_syntax_state_decimal(tkz, token, tkz->buffer,
                                                tkz->buffer + sizeof(tkz->buffer),
                                                data, end);
        }

        /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
        if (*data >= 0x30 && *data <= 0x39) {
            lxb_css_syntax_token_number(token)->have_sign = true;

            return lxb_css_syntax_state_decimal(tkz, token, tkz->buffer,
                                                tkz->buffer + sizeof(tkz->buffer),
                                                data, end);
        }

    push_delim:

        delim = lxb_css_syntax_list_append_delim(tkz, begin, begin + 1, '.');
        if (delim == NULL) {
            return NULL;
        }
    }

    token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

    lxb_css_syntax_token_base(token)->end = lxb_css_syntax_token_base(token)->begin + 1;
    lxb_css_syntax_token_delim(token)->character = '+';

    return data;
}

/*
 * U+002C COMMA (,)
 */
const lxb_char_t *
lxb_css_syntax_state_comma(lxb_css_syntax_tokenizer_t *tkz,
                           lxb_css_syntax_token_t *token,
                           const lxb_char_t *data, const lxb_char_t *end)
{
    token->type = LXB_CSS_SYNTAX_TOKEN_COMMA;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_base(token)->end = ++data;

    return data;
}

/*
 * U+002D HYPHEN-MINUS (-)
 */
const lxb_char_t *
lxb_css_syntax_state_minus(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                           const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_status_t status;

    lxb_css_syntax_token_base(token)->begin = data++;

    if (data >= end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

            lxb_css_syntax_token_base(token)->end = data;
            lxb_css_syntax_token_delim(token)->character = '-';

            return data;
        }
    }

    return lxb_css_syntax_state_minus_process(tkz, token, data, end);
}

const lxb_char_t *
lxb_css_syntax_state_minus_process(lxb_css_syntax_tokenizer_t *tkz,
                                   lxb_css_syntax_token_t *token,
                                   const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_char_t ch;
    lxb_status_t status;
    const lxb_char_t *begin, *second;
    lxb_css_syntax_token_t *delim;
    lxb_css_syntax_token_number_t *number;

    unsigned minuses_len = 1;
    static const lxb_char_t minuses[3] = "---";

    /* Check for <number-token> */

    /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
    if (*data >= 0x30 && *data <= 0x39) {
        data = lxb_css_syntax_state_consume_numeric(tkz, token, data, end);

        number = lxb_css_syntax_token_number(token);
        number->num = -number->num;

        lxb_css_syntax_token_number(token)->have_sign = true;

        return data;
    }

    /* U+002E FULL STOP (.) */
    if (*data == 0x2E) {
        begin = data++;

        if (data == end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                goto push_delim;
            }
        }

        /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
        if (*data >= 0x30 && *data <= 0x39) {
            data = lxb_css_syntax_state_decimal(tkz, token, tkz->buffer,
                                                tkz->buffer + sizeof(tkz->buffer),
                                                data, end);

            number = lxb_css_syntax_token_number(token);
            number->num = -number->num;

            lxb_css_syntax_token_number(token)->have_sign = true;

            return data;
        }

    push_delim:

        delim = lxb_css_syntax_list_append_delim(tkz, begin, begin + 1, '.');
        if (delim == NULL) {
            return NULL;
        }

        goto delim;
    }

    second = data;

    /* U+002D HYPHEN-MINUS (-) */
    if (*data == 0x2D) {
        data++;

        /* Check for <CDC-token> */

        if (data == end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                delim = lxb_css_syntax_list_append_delim(tkz, second,
                                                         second + 1, '-');
                if (delim == NULL) {
                    return NULL;
                }

                goto delim;
            }
        }

        if (*data == 0x2D) {
            LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, minuses, 3);
            return lxb_css_syntax_state_ident_like_not_url(tkz, token, ++data, end);
        }
        else if (*data == 0x3E) {
            token->type = LXB_CSS_SYNTAX_TOKEN_CDC;

            lxb_css_syntax_token_base(token)->end = ++data;

            return data;
        }

        minuses_len++;
    }

    /* Check for <ident-token> */

    if (lxb_css_syntax_res_name_map[*data] == LXB_CSS_SYNTAX_RES_NAME_START
        || *data == 0x00)
    {
        LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, minuses, minuses_len);

        return lxb_css_syntax_state_ident_like_not_url(tkz, token, data, end);
    }

    /* U+005C REVERSE SOLIDUS (\) */
    if (*data == 0x5C) {
        begin = data++;

        if (data == end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                goto delim_rev_solidus;
            }

            ch = *data;

            if (ch != 0x0A && ch != 0x0C && ch != 0x0D) {
                goto ident;
            }

            goto delim_rev_solidus;
        }

        ch = *data;

        if (ch != 0x0A && ch != 0x0C && ch != 0x0D) {
            goto ident;
        }

    delim_rev_solidus:

        delim = lxb_css_syntax_list_append_delim(tkz, begin, begin + 1, '\\');
        if (delim == NULL) {
            return NULL;
        }
    }

    if (minuses_len == 2) {
        delim = lxb_css_syntax_list_append_delim(tkz, second, NULL, '-');
        if (delim == NULL) {
            return NULL;
        }
    }

delim:

    token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

    lxb_css_syntax_token_base(token)->end = lxb_css_syntax_token_base(token)->begin + 1;
    lxb_css_syntax_token_delim(token)->character = '-';

    return data;

ident:

    LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, minuses, minuses_len);

    data = lxb_css_syntax_state_escaped(tkz, data, &end);
    if (data == NULL) {
        return NULL;
    }

    return lxb_css_syntax_state_ident_like_not_url(tkz, token, data, end);
}

/*
 * U+002E FULL STOP (.)
 */
const lxb_char_t *
lxb_css_syntax_state_full_stop(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                               const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_status_t status;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_number(token)->have_sign = false;

    data++;

    if (data >= end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            goto delim;
        }
    }

    /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
    if (*data >= 0x30 && *data <= 0x39) {
        return lxb_css_syntax_state_decimal(tkz, token, tkz->buffer,
                                            tkz->buffer + sizeof(tkz->buffer),
                                            data, end);
    }

delim:

    token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

    lxb_css_syntax_token_base(token)->end = lxb_css_syntax_token_base(token)->begin + 1;
    lxb_css_syntax_token_delim(token)->character = '.';

    return data;
}

/*
 * U+003A COLON (:)
 */
const lxb_char_t *
lxb_css_syntax_state_colon(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                           const lxb_char_t *data, const lxb_char_t *end)
{
    token->type = LXB_CSS_SYNTAX_TOKEN_COLON;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_base(token)->end = ++data;

    return data;
}

/*
 * U+003B SEMICOLON (;)
 */
const lxb_char_t *
lxb_css_syntax_state_semicolon(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                               const lxb_char_t *data, const lxb_char_t *end)
{
    token->type = LXB_CSS_SYNTAX_TOKEN_SEMICOLON;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_base(token)->end = ++data;

    return data;
}

/*
 * U+003C LESS-THAN SIGN (<)
 */
const lxb_char_t *
lxb_css_syntax_state_less_sign(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                               const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_char_t ch;
    lxb_status_t status;
    const lxb_char_t *mark, *minus, *esc;
    lxb_css_syntax_token_t *delim, *ident;

    lxb_css_syntax_token_base(token)->begin = data++;

    if ((end - data) > 2) {
        if (data[0] == '!' && data[1] == '-' && data[2] == '-') {
            data += 3;

            token->type = LXB_CSS_SYNTAX_TOKEN_CDO;
            lxb_css_syntax_token_base(token)->end = data;

            return data;
        }

        token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

        lxb_css_syntax_token_base(token)->end = data;
        lxb_css_syntax_token_delim(token)->character = '<';

        return data;
    }

    if (data >= end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            goto delim;
        }
    }

    /* U+0021 EXCLAMATION MARK */
    if (*data != 0x21) {
        goto delim;
    }

    mark = ++data;

    if (data == end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            goto delim_mark;
        }
    }

    /* U+002D HYPHEN-MINUS */
    if (*data != 0x2D) {
        goto delim_mark;
    }

    minus = ++data;

    if (data == end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            goto delim_minus;
        }
    }

    /* U+002D HYPHEN-MINUS */
    if (*data == 0x2D) {
        token->type = LXB_CSS_SYNTAX_TOKEN_CDO;

        lxb_css_syntax_token_base(token)->end = ++data;

        return data;
    }

    if (lxb_css_syntax_res_name_map[*data] == LXB_CSS_SYNTAX_RES_NAME_START) {
        goto ident;
    }

    /* U+005C REVERSE SOLIDUS (\) */
    if (*data == 0x5C) {
        esc = data++;

        if (data == end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                goto delim_esc;
            }

            ch = *data;

            if (ch != 0x0A && ch != 0x0C && ch != 0x0D) {
                ident = lxb_css_syntax_tokenizer_token_append(tkz);
                if (ident == NULL) {
                    return NULL;
                }

                lxb_css_syntax_token_base(ident)->begin = minus;

                LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                              (const lxb_char_t *) "-", 1);

                data = lxb_css_syntax_state_escaped(tkz, data, &end);
                if (data == NULL) {
                    return NULL;
                }

                data = lxb_css_syntax_state_ident_like_not_url(tkz, ident,
                                                               data, end);
                if (data == NULL) {
                    return NULL;
                }

                goto delim_mark;
            }

        delim_esc:

            delim = lxb_css_syntax_list_append_delim(tkz, esc, esc + 1, '\\');
            if (delim == NULL) {
                return NULL;
            }

            goto delim_minus;
        }

        ch = *data--;

        if (ch == 0x0A || ch == 0x0C || ch == 0x0D) {
            goto delim_minus;
        }

        data = lxb_css_syntax_state_escaped(tkz, data, &end);
        if (data == NULL) {
            return NULL;
        }
    }
    else if (*data != 0x00) {
        delim = lxb_css_syntax_list_append_delim(tkz, minus - 1, NULL, '-');
        if (delim == NULL) {
            return NULL;
        }

        goto delim_mark;
    }

ident:

    ident = lxb_css_syntax_tokenizer_token_append(tkz);
    if (ident == NULL) {
        return NULL;
    }

    lxb_css_syntax_token_base(ident)->begin = minus;

    LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, (const lxb_char_t *) "-", 1);

    data = lxb_css_syntax_state_ident_like_not_url(tkz, ident, data, end);
    if (data == NULL) {
        return NULL;
    }

    goto delim_mark;

delim_minus:

    delim = lxb_css_syntax_list_append_delim(tkz, minus - 1, minus, '-');
    if (delim == NULL) {
        return NULL;
    }

delim_mark:

    delim = lxb_css_syntax_list_append_delim(tkz, mark - 1, mark, '!');
    if (delim == NULL) {
        return NULL;
    }

delim:

    token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

    lxb_css_syntax_token_base(token)->end = lxb_css_syntax_token_base(token)->begin + 1;
    lxb_css_syntax_token_delim(token)->character = '<';

    return data;
}

/*
 * U+0040 COMMERCIAL AT (@)
 */
const lxb_char_t *
lxb_css_syntax_state_at(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                        const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_char_t ch;
    lxb_status_t status;
    const lxb_char_t *minus, *esc;
    lxb_css_syntax_token_t *delim;

    unsigned minuses_len = 0;
    static const lxb_char_t minuses[2] = "--";

    token->type = LXB_CSS_SYNTAX_TOKEN_AT_KEYWORD;

    lxb_css_syntax_token_base(token)->begin = data++;

    if (data >= end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            goto delim;
        }
    }

    if (lxb_css_syntax_res_name_map[*data] == LXB_CSS_SYNTAX_RES_NAME_START) {
        return lxb_css_syntax_state_consume_ident(tkz, token, data, end);
    }

    minus = data;

    /* U+002D HYPHEN-MINUS */
    if (*data == 0x2D) {
        data++;

        if (data == end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                delim = lxb_css_syntax_list_append_delim(tkz, minus,
                                                         minus + 1, '-');
                if (delim == NULL) {
                    return NULL;
                }

                goto delim;
            }
        }

        if (lxb_css_syntax_res_name_map[*data] == LXB_CSS_SYNTAX_RES_NAME_START
            || *data == 0x00)
        {
            LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, minuses, 1);
            return lxb_css_syntax_state_consume_ident(tkz, token, data, end);
        }
        else if (*data == 0x2D) {
            LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, minuses, 2);
            return lxb_css_syntax_state_consume_ident(tkz, token, ++data, end);
        }

        minuses_len++;
    }

    /* U+005C REVERSE SOLIDUS (\) */
    if (*data == 0x5C) {
        esc = ++data;

        if (data == end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                goto delim_esc;
            }
        }

        ch = *data;

        if (ch != 0x0A && ch != 0x0C && ch != 0x0D) {
            LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, minuses, minuses_len);

            data = lxb_css_syntax_state_escaped(tkz, data, &end);
            if (data == NULL) {
                return NULL;
            }

            return lxb_css_syntax_state_consume_ident(tkz, token, data, end);
        }

        goto delim_esc;
    }
    else if (*data != 0x00) {
        goto delim_minus;
    }

    LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, minuses, minuses_len);

    return lxb_css_syntax_state_consume_ident(tkz, token, data, end);

delim_esc:

    delim = lxb_css_syntax_list_append_delim(tkz, esc - 1, esc, '\\');
    if (delim == NULL) {
        return NULL;
    }

delim_minus:

    if (minuses_len != 0) {
        delim = lxb_css_syntax_list_append_delim(tkz, minus, NULL, '-');
        if (delim == NULL) {
            return NULL;
        }
    }

delim:

    token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

    lxb_css_syntax_token_base(token)->end = lxb_css_syntax_token_base(token)->begin + 1;
    lxb_css_syntax_token_delim(token)->character = '@';

    return data;
}

/*
 * U+005B LEFT SQUARE BRACKET ([)
 */
const lxb_char_t *
lxb_css_syntax_state_ls_bracket(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                                const lxb_char_t *data, const lxb_char_t *end)
{
    token->type = LXB_CSS_SYNTAX_TOKEN_LS_BRACKET;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_base(token)->end = ++data;

    return data;
}

/*
 * U+005C REVERSE SOLIDUS (\)
 */
const lxb_char_t *
lxb_css_syntax_state_rsolidus(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                              const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_char_t ch;
    lxb_status_t status;

    lxb_css_syntax_token_base(token)->begin = data++;

    if (data >= end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            goto delim;
        }
    }

    ch = *data;

    if (ch == 0x0A || ch == 0x0C || ch == 0x0D) {
        goto delim;
    }

    data = lxb_css_syntax_state_escaped(tkz, data, &end);
    if (data == NULL) {
        return NULL;
    }

    return lxb_css_syntax_state_ident_like(tkz, token, data, end);

delim:

    token->type = LXB_CSS_SYNTAX_TOKEN_DELIM;

    lxb_css_syntax_token_base(token)->end = lxb_css_syntax_token_base(token)->begin + 1;
    lxb_css_syntax_token_delim(token)->character = '\\';

    return data;
}

/*
 * U+005D RIGHT SQUARE BRACKET (])
 */
const lxb_char_t *
lxb_css_syntax_state_rs_bracket(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                                const lxb_char_t *data, const lxb_char_t *end)
{
    token->type = LXB_CSS_SYNTAX_TOKEN_RS_BRACKET;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_base(token)->end = ++data;

    return data;
}

/*
 * U+007B LEFT CURLY BRACKET ({)
 */
const lxb_char_t *
lxb_css_syntax_state_lc_bracket(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                                const lxb_char_t *data, const lxb_char_t *end)
{
    token->type = LXB_CSS_SYNTAX_TOKEN_LC_BRACKET;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_base(token)->end = ++data;

    return data;
}

/*
 * U+007D RIGHT CURLY BRACKET (})
 */
const lxb_char_t *
lxb_css_syntax_state_rc_bracket(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                                const lxb_char_t *data, const lxb_char_t *end)
{
    token->type = LXB_CSS_SYNTAX_TOKEN_RC_BRACKET;

    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_base(token)->end = ++data;

    return data;
}

/*
 * Numeric
 */
lxb_inline void
lxb_css_syntax_consume_numeric_set_int(lxb_css_syntax_tokenizer_t *tkz,
                                       lxb_css_syntax_token_t *token,
                                       const lxb_char_t *start, const lxb_char_t *end)
{
    double num = lexbor_strtod_internal(start, (end - start), 0);

    token->type = LXB_CSS_SYNTAX_TOKEN_NUMBER;

    lxb_css_syntax_token_number(token)->is_float = false;
    lxb_css_syntax_token_number(token)->num = num;
}

lxb_inline void
lxb_css_syntax_consume_numeric_set_float(lxb_css_syntax_tokenizer_t *tkz,
                                         lxb_css_syntax_token_t *token,
                                         const lxb_char_t *start, const lxb_char_t *end,
                                         bool e_is_negative, int exponent, int e_digit)
{
    if (e_is_negative) {
        exponent -= e_digit;
    }
    else {
        exponent += e_digit;
    }

    double num = lexbor_strtod_internal(start, (end - start), exponent);

    token->type = LXB_CSS_SYNTAX_TOKEN_NUMBER;

    lxb_css_syntax_token_number(token)->num = num;
    lxb_css_syntax_token_number(token)->is_float = true;
}

const lxb_char_t *
lxb_css_syntax_state_consume_before_numeric(lxb_css_syntax_tokenizer_t *tkz,
                                            lxb_css_syntax_token_t *token,
                                            const lxb_char_t *data,
                                            const lxb_char_t *end)
{
    lxb_css_syntax_token_base(token)->begin = data;
    lxb_css_syntax_token_number(token)->have_sign = false;

    return lxb_css_syntax_state_consume_numeric(tkz, token, data, end);
}

static const lxb_char_t *
lxb_css_syntax_state_consume_numeric(lxb_css_syntax_tokenizer_t *tkz,
                                     lxb_css_syntax_token_t *token,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end)
{
    lxb_status_t status;
    lxb_css_syntax_token_t *delim;

    lxb_char_t *buf_start = tkz->buffer;
    lxb_char_t *buf_end = buf_start + sizeof(tkz->buffer);

    do {
        /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
        if (*data < 0x30 || *data > 0x39) {
            break;
        }

        if (buf_start != buf_end) {
            *buf_start++ = *data;
        }

        if (++data == end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                lxb_css_syntax_token_base(token)->end = data;

                lxb_css_syntax_consume_numeric_set_int(tkz, token, tkz->buffer,
                                                       buf_start);
                return data;
            }
        }
    }
    while (true);

    lxb_css_syntax_token_base(token)->end = data;

    /* U+002E FULL STOP (.) */
    if (*data != 0x2E) {
        lxb_css_syntax_consume_numeric_set_int(tkz, token, tkz->buffer,
                                               buf_start);

        return lxb_css_syntax_state_consume_numeric_name_start(tkz, token,
                                                               data, end);
    }

    data++;

    if (data == end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            goto delim;
        }
    }

    if (*data >= 0x30 && *data <= 0x39) {
        return lxb_css_syntax_state_decimal(tkz, token, buf_start, buf_end,
                                            data, end);
    }

delim:

    lxb_css_syntax_consume_numeric_set_int(tkz, token, tkz->buffer, buf_start);

    delim = lxb_css_syntax_list_append_delim(tkz, data - 1, data, '.');
    if (delim == NULL) {
        return NULL;
    }

    return data;
}

static const lxb_char_t *
lxb_css_syntax_state_decimal(lxb_css_syntax_tokenizer_t *tkz,
                             lxb_css_syntax_token_t *token,
                             lxb_char_t *buf_start, lxb_char_t *buf_end,
                             const lxb_char_t *data, const lxb_char_t *end)
{
    bool e_is_negative;
    int exponent, e_digit;
    lxb_char_t ch, by;
    lxb_status_t status;
    const lxb_char_t *last;
    lxb_css_syntax_token_t *delim, *t_str;
    lxb_css_syntax_token_string_t *str;

    exponent = 0;

    str = lxb_css_syntax_token_dimension_string(token);
    t_str = (lxb_css_syntax_token_t *) (void *) str;

    /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
    do {
        if (buf_start != buf_end) {
            *buf_start++ = *data;
            exponent -= 1;
        }

        data++;

        if (data >= end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                lxb_css_syntax_token_base(token)->end = data;

                lxb_css_syntax_consume_numeric_set_float(tkz, token, tkz->buffer,
                                                         buf_start, 0, exponent, 0);
                return data;
            }
        }
    }
    while (*data >= 0x30 && *data <= 0x39);

    lxb_css_syntax_token_base(token)->end = data;
    lxb_css_syntax_token_base(str)->begin = data;

    ch = *data;

    /* U+0045 Latin Capital Letter (E) or U+0065 Latin Small Letter (e) */
    if (ch != 0x45 && ch != 0x65) {
        lxb_css_syntax_consume_numeric_set_float(tkz, token, tkz->buffer,
                                                 buf_start, 0, exponent, 0);

        return lxb_css_syntax_state_consume_numeric_name_start(tkz, token,
                                                               data, end);
    }

    e_digit = 0;
    e_is_negative = false;

    lxb_css_syntax_token_base(str)->end = ++data;

    if (data == end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            lxb_css_syntax_consume_numeric_set_float(tkz, token, tkz->buffer,
                                                     buf_start, 0, exponent, 0);

            LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, &ch, 1);

            token->type = LXB_CSS_SYNTAX_TOKEN_DIMENSION;

            return lxb_css_syntax_state_dimension_set(tkz, token, data);
        }
    }

    switch (*data) {
        /* U+002D HYPHEN-MINUS (-) */
        case 0x2D:
            e_is_negative = true;
            /* fall through */

        /* U+002B PLUS SIGN (+) */
        case 0x2B:
            last = data++;
            by = *last;

            if (data == end) {
                LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
                if (data >= end) {
                    goto dimension;
                }
            }

            /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
            if (*data < 0x30 || *data > 0x39) {
                goto dimension;
            }

            break;

        default:
            /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
            if (*data < 0x30 || *data > 0x39) {
                lxb_css_syntax_consume_numeric_set_float(tkz, token,
                                                         tkz->buffer, buf_start,
                                                         0, exponent, 0);

                token->type = LXB_CSS_SYNTAX_TOKEN_DIMENSION;

                LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, &ch, 1);

                return lxb_css_syntax_state_consume_ident(tkz, t_str,
                                                          data, end);
            }

            break;
    }

    /* U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9) */
    do {
        e_digit = (*data - 0x30) + e_digit * 0x0A;

        if (++data == end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                lxb_css_syntax_token_base(token)->end = data;

                lxb_css_syntax_consume_numeric_set_float(tkz, token, tkz->buffer, buf_start,
                                                         e_is_negative, exponent, e_digit);
                return data;
            }
        }
    }
    while(*data >= 0x30 && *data <= 0x39);

    lxb_css_syntax_consume_numeric_set_float(tkz, token, tkz->buffer, buf_start,
                                             e_is_negative, exponent, e_digit);

    return lxb_css_syntax_state_consume_numeric_name_start(tkz, token,
                                                           data, end);

dimension:

    lxb_css_syntax_consume_numeric_set_float(tkz, token,
                                             tkz->buffer, buf_start,
                                             0, exponent, 0);

    token->type = LXB_CSS_SYNTAX_TOKEN_DIMENSION;

    LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, &ch, 1);

    if (by == '-') {
        LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status, &by, 1);

        return lxb_css_syntax_state_consume_ident(tkz, t_str, data, end);
    }

    delim = lxb_css_syntax_list_append_delim(tkz, last, NULL, '+');
    if (delim == NULL) {
        return NULL;
    }

    return lxb_css_syntax_state_dimension_set(tkz, token, data);
}

static const lxb_char_t *
lxb_css_syntax_state_consume_numeric_name_start(lxb_css_syntax_tokenizer_t *tkz,
                                                lxb_css_syntax_token_t *token,
                                                const lxb_char_t *data,
                                                const lxb_char_t *end)
{
    bool have_minus;
    lxb_char_t ch;
    lxb_status_t status;
    const lxb_char_t *esc, *minus;
    lxb_css_syntax_token_t *delim, *t_str;
    lxb_css_syntax_token_string_t *str;

    str = lxb_css_syntax_token_dimension_string(token);
    t_str = (lxb_css_syntax_token_t *) (void *) str;

    lxb_css_syntax_token_base(t_str)->begin = data;

    ch = *data;

    if (lxb_css_syntax_res_name_map[ch] == LXB_CSS_SYNTAX_RES_NAME_START
        || ch == 0x00)
    {
        token->type = LXB_CSS_SYNTAX_TOKEN_DIMENSION;

        return lxb_css_syntax_state_consume_ident(tkz, t_str, data, end);
    }

    /* U+0025 PERCENTAGE SIGN (%) */
    if (ch == 0x25) {
        token->type = LXB_CSS_SYNTAX_TOKEN_PERCENTAGE;

        lxb_css_syntax_token_base(token)->end = ++data;

        return data;
    }

    have_minus = false;
    minus = data;

    /* U+002D HYPHEN-MINUS */
    if (ch == 0x2D) {
        data++;

        if (data >= end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                delim = lxb_css_syntax_list_append_delim(tkz, data - 1,
                                                         data, '-');
                if (delim == NULL) {
                    return NULL;
                }

                return data;
            }
        }

        ch = *data;

        if (lxb_css_syntax_res_name_map[ch] == LXB_CSS_SYNTAX_RES_NAME_START
            || ch == 0x2D || ch == 0x00)
        {
            token->type = LXB_CSS_SYNTAX_TOKEN_DIMENSION;

            LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                          (const lxb_char_t *) "-", 1);

            return lxb_css_syntax_state_consume_ident(tkz, t_str, data, end);
        }

        have_minus = true;
    }

    esc = data;

    /* U+005C REVERSE SOLIDUS (\) */
    if (ch == 0x5C) {
        data++;

        if (data >= end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                goto delim_rev_solidus;
            }
        }

        ch = *data;

        if (ch != 0x0A && ch != 0x0C && ch != 0x0D) {
            token->type = LXB_CSS_SYNTAX_TOKEN_DIMENSION;

            if (have_minus) {
                LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                              (const lxb_char_t *) "-", 1);
            }

            data = lxb_css_syntax_state_escaped(tkz, data, &end);
            if (data == NULL) {
                return NULL;
            }

            return lxb_css_syntax_state_consume_ident(tkz, t_str, data, end);
        }

    delim_rev_solidus:

        delim = lxb_css_syntax_list_append_delim(tkz, esc, esc + 1, '\\');
        if (delim == NULL) {
            return NULL;
        }

        if (have_minus) {
            delim = lxb_css_syntax_list_append_delim(tkz, minus,
                                                     minus + 1, '-');
            if (delim == NULL) {
                return NULL;
            }
        }

        return data;
    }

    lxb_css_syntax_token_base(token)->end = minus;

    if (have_minus) {
        delim = lxb_css_syntax_list_append_delim(tkz, minus, NULL, '-');
        if (delim == NULL) {
            return NULL;
        }
    }

    return data;
}

static const lxb_char_t *
lxb_css_syntax_state_consume_ident(lxb_css_syntax_tokenizer_t *tkz,
                                   lxb_css_syntax_token_t *token,
                                   const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_status_t status;
    const lxb_char_t *begin, *last;
    lxb_css_syntax_token_t *delim;

    begin = data;

    for (;; data++) {
        if (data >= end) {
            if (begin < data) {
                LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
            }

            last = data;

            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                lxb_css_syntax_token_base(token)->end = last;

                return lxb_css_syntax_state_string_set(tkz, token, data);
            }

            begin = data;
        }

        if (lxb_css_syntax_res_name_map[*data] == 0x00) {

            /* U+005C REVERSE SOLIDUS (\) */
            if (*data == 0x5C) {
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                begin = data;
                last = ++data;

                if (data == end) {
                    LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
                    if (data >= end) {
                        goto push_delim_last;
                    }
                }

                if (*data == 0x0A || *data == 0x0C || *data == 0x0D) {
                    goto push_delim_last;
                }

                data = lxb_css_syntax_state_escaped(tkz, data, &end);
                if (data == NULL) {
                    return NULL;
                }

                begin = data--;
            }
            else if (*data == 0x00) {
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                              lexbor_str_res_ansi_replacement_character,
                                              sizeof(lexbor_str_res_ansi_replacement_character) - 1);
                begin = data + 1;
            }
            else {
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                lxb_css_syntax_token_base(token)->end = data;

                return lxb_css_syntax_state_string_set(tkz, token, data);
            }
        }
    }

    return data;

push_delim_last:

    lxb_css_syntax_token_base(token)->end = begin;

    delim = lxb_css_syntax_list_append_delim(tkz, begin, last, '\\');
    if (delim == NULL) {
        return NULL;
    }

    return lxb_css_syntax_state_string_set(tkz, token, data);
}

const lxb_char_t *
lxb_css_syntax_state_ident_like_begin(lxb_css_syntax_tokenizer_t *tkz,
                                      lxb_css_syntax_token_t *token,
                                      const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_css_syntax_token_base(token)->begin = data;

    return lxb_css_syntax_state_ident_like(tkz, token, data, end);
}

const lxb_char_t *
lxb_css_syntax_state_ident_like(lxb_css_syntax_tokenizer_t *tkz,
                                lxb_css_syntax_token_t *token,
                                const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_char_t ch;
    lxb_status_t status;
    const lxb_char_t *begin, *ws_begin;
    lxb_css_syntax_token_t *ws;
    lxb_css_syntax_token_string_t *str, *ws_str;
    static const lxb_char_t url[] = "url";

    data = lxb_css_syntax_state_consume_ident(tkz, token, data, end);

    if (data >= end) {
        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
        if (data >= end) {
            token->type = LXB_CSS_SYNTAX_TOKEN_IDENT;
            return data;
        }
    }

    if (data < end && *data == '(') {
        lxb_css_syntax_token_base(token)->end = ++data;

        str = lxb_css_syntax_token_string(token);

        if (str->length == 3 && lexbor_str_data_casecmp(str->data, url)) {
            begin = data;

            tkz->pos += str->length + 1;
            ws_begin = tkz->pos;

            do {
                if (data >= end) {
                    if (begin < data) {
                        LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                    }

                    LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
                    if (data >= end) {
                        begin = data;
                        goto with_ws;
                    }

                    begin = data;
                }

                ch = *data;

                if (lexbor_utils_whitespace(ch, !=, &&)) {
                    /* U+0022 QUOTATION MARK (") or U+0027 APOSTROPHE (') */
                    if (ch == 0x22 || ch == 0x27) {
                        goto with_ws;
                    }

                    tkz->pos = tkz->start;

                    return lxb_css_syntax_state_url(tkz, token, data, end);
                }

                data++;
            }
            while (true);
        }

        token->type = LXB_CSS_SYNTAX_TOKEN_FUNCTION;

        return data;
    }

    token->type = LXB_CSS_SYNTAX_TOKEN_IDENT;

    return data;

with_ws:

    token->type = LXB_CSS_SYNTAX_TOKEN_FUNCTION;

    if (ws_begin != tkz->pos || begin < data) {
        if (begin < data) {
            LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
        }

        if (tkz->pos >= tkz->end) {
            if (lxb_css_syntax_string_realloc(tkz, 1024) != LXB_STATUS_OK) {
                return NULL;
            }
        }

        str->data = tkz->start;
        *tkz->pos = 0x00;

        ws = lxb_css_syntax_tokenizer_token_append(tkz);
        if (ws == NULL) {
            return NULL;
        }

        ws->type = LXB_CSS_SYNTAX_TOKEN_WHITESPACE;

        lxb_css_syntax_token_base(ws)->begin = begin;
        lxb_css_syntax_token_base(ws)->end = data;

        ws_str = lxb_css_syntax_token_string(ws);

        ws_str->data = tkz->start + str->length + 1;
        ws_str->length = tkz->pos - ws_str->data;
    }

    tkz->pos = tkz->start;

    return data;
}

const lxb_char_t *
lxb_css_syntax_state_ident_like_not_url_begin(lxb_css_syntax_tokenizer_t *tkz,
                                              lxb_css_syntax_token_t *token,
                                              const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_css_syntax_token_base(token)->begin = data;

    return lxb_css_syntax_state_ident_like_not_url(tkz, token, data, end);
}

const lxb_char_t *
lxb_css_syntax_state_ident_like_not_url(lxb_css_syntax_tokenizer_t *tkz,
                                        lxb_css_syntax_token_t *token,
                                        const lxb_char_t *data, const lxb_char_t *end)
{
    data = lxb_css_syntax_state_consume_ident(tkz, token, data, end);
    if (data == NULL) {
        return NULL;
    }

    if (data < end && *data == '(') {
        token->type = LXB_CSS_SYNTAX_TOKEN_FUNCTION;

        lxb_css_syntax_token_base(token)->end = ++data;

        return data;
    }

    token->type = LXB_CSS_SYNTAX_TOKEN_IDENT;

    return data;
}

/*
 * URL
 */
static const lxb_char_t *
lxb_css_syntax_state_url(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                         const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_char_t ch;
    lxb_status_t status;
    const lxb_char_t *begin;

    status = LXB_STATUS_OK;

    *tkz->pos = 0x00;

    begin = data;

    do {
        if (data >= end) {
            if (begin < data) {
                LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
            }

            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                lxb_css_syntax_tokenizer_error_add(tkz->parse_errors, data,
                                                   LXB_CSS_SYNTAX_TOKENIZER_ERROR_EOINUR);

                token->type = LXB_CSS_SYNTAX_TOKEN_URL;

                lxb_css_syntax_token_base(token)->end = data;

                return lxb_css_syntax_state_string_set(tkz, token, data);
            }

            begin = data;
        }

        switch (*data) {
            /* U+0000 NULL (\0) */
            case 0x00:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                LXB_CSS_SYNTAX_STR_APPEND_LEN(tkz, status,
                                              lexbor_str_res_ansi_replacement_character,
                                              sizeof(lexbor_str_res_ansi_replacement_character) - 1);
                begin = data + 1;
                break;

            /* U+0029 RIGHT PARENTHESIS ()) */
            case 0x29:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                token->type = LXB_CSS_SYNTAX_TOKEN_URL;

                lxb_css_syntax_token_base(token)->end = ++data;

                return lxb_css_syntax_state_string_set(tkz, token, data);

            /*
             * U+0022 QUOTATION MARK (")
             * U+0027 APOSTROPHE (')
             * U+0028 LEFT PARENTHESIS (()
             * U+000B LINE TABULATION
             * U+007F DELETE
             */
            case 0x22:
            case 0x27:
            case 0x28:
            case 0x0B:
            case 0x7F:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                lxb_css_syntax_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_CSS_SYNTAX_TOKENIZER_ERROR_QOINUR);

                return lxb_css_syntax_state_bad_url(tkz, token, data + 1, end);

            /* U+005C REVERSE SOLIDUS (\) */
            case 0x5C:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                begin = ++data;

                if (data == end) {
                    LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
                    if (data >= end) {
                        lxb_css_syntax_tokenizer_error_add(tkz->parse_errors, data,
                                                           LXB_CSS_SYNTAX_TOKENIZER_ERROR_WRESINUR);

                        token->type = LXB_CSS_SYNTAX_TOKEN_BAD_URL;

                        lxb_css_syntax_token_base(token)->end = begin;

                        return lxb_css_syntax_state_string_set(tkz, token, data);
                    }
                }

                ch = *data;

                if (ch == 0x0A || ch == 0x0C || ch == 0x0D) {
                    lxb_css_syntax_tokenizer_error_add(tkz->parse_errors, data,
                                       LXB_CSS_SYNTAX_TOKENIZER_ERROR_WRESINUR);

                    lxb_css_syntax_token_base(token)->end = data;

                    return lxb_css_syntax_state_bad_url(tkz, token, data, end);
                }

                data = lxb_css_syntax_state_escaped(tkz, data, &end);
                if (data == NULL) {
                    return NULL;
                }

                begin = data--;

                break;

            /*
             * U+0009 CHARACTER TABULATION (tab)
             * U+000A LINE FEED (LF)
             * U+000C FORM FEED (FF)
             * U+000D CARRIAGE RETURN (CR)
             * U+0020 SPACE
             */
            case 0x09:
            case 0x0A:
            case 0x0C:
            case 0x0D:
            case 0x20:
                if (begin < data) {
                    LXB_CSS_SYNTAX_STR_APPEND(tkz, status, begin, data);
                }

                lxb_css_syntax_token_base(token)->end = data;

                begin = ++data;

                do {
                    if (data == end) {
                        LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
                        if (data >= end) {
                            lxb_css_syntax_tokenizer_error_add(tkz->parse_errors, data,
                                                               LXB_CSS_SYNTAX_TOKENIZER_ERROR_EOINUR);

                            token->type = LXB_CSS_SYNTAX_TOKEN_BAD_URL;

                            lxb_css_syntax_token_base(token)->end = begin;

                            return lxb_css_syntax_state_string_set(tkz, token, data);
                        }
                    }

                    ch = *data;

                    if (lexbor_utils_whitespace(ch, !=, &&)) {
                        /* U+0029 RIGHT PARENTHESIS ()) */
                        if (*data == 0x29) {
                            token->type = LXB_CSS_SYNTAX_TOKEN_URL;

                            lxb_css_syntax_token_base(token)->end = ++data;

                            return lxb_css_syntax_state_string_set(tkz, token, data);
                        }

                        return lxb_css_syntax_state_bad_url(tkz, token,
                                                            data, end);
                    }

                    data++;
                }
                while (true);

            default:
                /*
                 * Inclusive:
                 * U+0000 NULL and U+0008 BACKSPACE or
                 * U+000E SHIFT OUT and U+001F INFORMATION SEPARATOR ONE
                 */
                if ((*data >= 0x00 && *data <= 0x08)
                    || (*data >= 0x0E && *data <= 0x1F))
                {
                    lxb_css_syntax_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_CSS_SYNTAX_TOKENIZER_ERROR_QOINUR);

                    return lxb_css_syntax_state_bad_url(tkz, token,
                                                        data + 1, end);
                }

                break;
        }

        data++;
    }
    while (true);

    return data;
}

/*
 * Bad URL
 */
static const lxb_char_t *
lxb_css_syntax_state_bad_url(lxb_css_syntax_tokenizer_t *tkz, lxb_css_syntax_token_t *token,
                               const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_status_t status;

    token->type = LXB_CSS_SYNTAX_TOKEN_BAD_URL;

    if(lxb_css_syntax_state_string_set(tkz, token, data) == NULL) {
        return NULL;
    }

    do {
        if (data >= end) {
            LXB_CSS_SYNTAX_NEXT_CHUNK(tkz, status, data, end);
            if (data >= end) {
                lxb_css_syntax_token_base(token)->end = data;
                return data;
            }
        }

        /* U+0029 RIGHT PARENTHESIS ()) */
        if (*data == 0x29) {
            lxb_css_syntax_token_base(token)->end = ++data;
            return data;
        }
        /* U+005C REVERSE SOLIDUS (\) */
        else if (*data == 0x5C) {
            data++;
        }

        data++;
    }
    while (true);

    return data;
}

lxb_inline lxb_status_t
lxb_css_syntax_string_append_rep(lxb_css_syntax_tokenizer_t *tkz)
{
    return lxb_css_syntax_string_append(tkz, lexbor_str_res_ansi_replacement_character,
                                        sizeof(lexbor_str_res_ansi_replacement_character) - 1);
}

static const lxb_char_t *
lxb_css_syntax_state_escaped(lxb_css_syntax_tokenizer_t *tkz,
                             const lxb_char_t *data, const lxb_char_t **end)
{
    uint32_t cp;
    unsigned count;
    lxb_status_t status;

    cp = 0;

    for (count = 0; count < 6; count++, data++) {
        if (data >= *end) {
            status = lxb_css_syntax_tokenizer_next_chunk(tkz, &data, end);
            if (status != LXB_STATUS_OK) {
                return NULL;
            }

            if (data >= *end) {
                if (count == 0) {
                    return *end;
                }

                break;
            }
        }

        if (lexbor_str_res_map_hex[*data] == 0xFF) {
            if (count == 0) {
                if (*data == 0x00) {
                    status = lxb_css_syntax_string_append_rep(tkz);
                    if (status != LXB_STATUS_OK) {
                        return NULL;
                    }

                    return data + 1;
                }

                status = lxb_css_syntax_string_append(tkz, data, 1);
                if (status != LXB_STATUS_OK) {
                    return NULL;
                }

                return data + 1;
            }

            switch (*data) {
                case 0x0D:
                    data++;

                    status = lxb_css_syntax_tokenizer_next_chunk(tkz, &data,
                                                                 end);
                    if (status != LXB_STATUS_OK) {
                        return NULL;
                    }

                    if (data >= *end) {
                        break;
                    }

                    if (*data == 0x0A) {
                        data++;
                    }

                    break;

                case 0x09:
                case 0x20:
                case 0x0A:
                case 0x0C:
                    data++;
                    break;
            }

            break;
        }

        cp <<= 4;
        cp |= lexbor_str_res_map_hex[*data];
    }

    if ((tkz->end - tkz->pos) < 5) {
        if (lxb_css_syntax_string_realloc(tkz, 1024) != LXB_STATUS_OK) {
            return NULL;
        }
    }

    lxb_css_syntax_codepoint_to_ascii(tkz, cp);

    return data;
}

static const lxb_char_t *
lxb_css_syntax_state_escaped_string(lxb_css_syntax_tokenizer_t *tkz,
                                    const lxb_char_t *data, const lxb_char_t **end)
{
    lxb_status_t status;

    /* U+000D CARRIAGE RETURN */
    if (*data == 0x0D) {
        data++;

        if (data >= *end) {
            status = lxb_css_syntax_tokenizer_next_chunk(tkz, &data, end);
            if (status != LXB_STATUS_OK) {
                return NULL;
            }

            if (data >= *end) {
                return data;
            }
        }

        /* U+000A LINE FEED */
        if (*data == 0x0A) {
            data++;
        }

        return data;
    }

    if (*data == 0x00) {
        status = lxb_css_syntax_string_append_rep(tkz);
        if (status != LXB_STATUS_OK) {
            return NULL;
        }

        return data + 1;
    }

    if (*data == 0x0A || *data == 0x0C) {
        return data + 1;
    }

    return lxb_css_syntax_state_escaped(tkz, data, end);
}
