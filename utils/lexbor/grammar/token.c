/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/grammar/token.h"
#include "lexbor/grammar/tokenizer.h"

#include "lexbor/core/conv.h"
#include "lexbor/html/serialize.h"


lxb_grammar_token_t *
lxb_grammar_token_create(lxb_grammar_tokenizer_t *tkz,
                         lxb_grammar_token_type_t type)
{
    lxb_grammar_token_t *token;
    lxb_grammar_document_t *document;

    document = tkz->html_parser->tree->document;

    token = lexbor_mraw_calloc(document->dom_document.mraw,
                               sizeof(lxb_grammar_token_t));
    if (token == NULL) {
        return NULL;
    }

    token->type = type;

    return token;
}

lxb_grammar_token_t *
lxb_grammar_token_destroy(lxb_grammar_tokenizer_t *tkz,
                          lxb_grammar_token_t *token)
{
    lxb_grammar_document_t *document;

    document = tkz->html_parser->tree->document;

    return lexbor_mraw_free(document->dom_document.mraw, token);
}

lxb_status_t
lxb_grammar_token_serialize(lxb_grammar_token_t *token,
                            lxb_grammar_serialize_cb_f func, void *ctx)
{
    size_t len;
    lxb_char_t buf[128];
    lxb_status_t status;

    switch (token->type) {
        case LXB_GRAMMAR_TOKEN_WHITESPACE:
        case LXB_GRAMMAR_TOKEN_UNQUOTED:
        case LXB_GRAMMAR_TOKEN_DELIM:
            return func(token->u.str.data, token->u.str.length, ctx);

        case LXB_GRAMMAR_TOKEN_ELEMENT:
            return lxb_html_serialize_cb(token->u.node, func, ctx);

        case LXB_GRAMMAR_TOKEN_EQUALS:
            return func((lxb_char_t *) "=", 1, ctx);

        case LXB_GRAMMAR_TOKEN_STRING:
            status = func((lxb_char_t *) "\"", 1, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            status = func(token->u.str.data, token->u.str.length, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            return func((lxb_char_t *) "\"", 1, ctx);

        case LXB_GRAMMAR_TOKEN_NUMBER:
            len = lexbor_conv_float_to_data(token->u.num, buf,
                                            (sizeof(buf) / sizeof(lxb_char_t)));
            return func(buf, len, ctx);

        case LXB_GRAMMAR_TOKEN_ASTERISK:
            return func((lxb_char_t *) "*", 1, ctx);

        case LXB_GRAMMAR_TOKEN_PLUS:
            return func((lxb_char_t *) "+", 1, ctx);

        case LXB_GRAMMAR_TOKEN_EXCLAMATION:
            return func((lxb_char_t *) "!", 1, ctx);

        case LXB_GRAMMAR_TOKEN_QUESTION:
            return func((lxb_char_t *) "?", 1, ctx);

        case LXB_GRAMMAR_TOKEN_HASH:
            return func((lxb_char_t *) "#", 1, ctx);

        case LXB_GRAMMAR_TOKEN_EXCLUDE_WS:
            return func((lxb_char_t *) "^WS", 3, ctx);

        case LXB_GRAMMAR_TOKEN_EXCLUDE_SORT:
            return func((lxb_char_t *) "^SORT", 5, ctx);

        case LXB_GRAMMAR_TOKEN_COUNT:
            status = func((lxb_char_t *) "{", 1, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            len = lexbor_conv_float_to_data((double) token->u.count, buf,
                                            (sizeof(buf) / sizeof(lxb_char_t)));
            status = func(buf, len, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            return func((lxb_char_t *) "}", 1, ctx);

        case LXB_GRAMMAR_TOKEN_RANGE:
            status = func((lxb_char_t *) "{", 1, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            len = lexbor_conv_float_to_data((double) token->u.period.start, buf,
                                            (sizeof(buf) / sizeof(lxb_char_t)));
            status = func(buf, len, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            status = func((lxb_char_t *) ",", 1, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            if (token->u.period.stop >= 0) {
                len = lexbor_conv_float_to_data((double) token->u.period.stop, buf,
                                                (sizeof(buf) / sizeof(lxb_char_t)));
                status = func(buf, len, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }
            }

            return func((lxb_char_t *) "}", 1, ctx);

        case LXB_GRAMMAR_TOKEN_BAR:
            return func((lxb_char_t *) "|", 1, ctx);

        case LXB_GRAMMAR_TOKEN_DOUBLE_BAR:
            return func((lxb_char_t *) "||", 2, ctx);

        case LXB_GRAMMAR_TOKEN_AND:
            return func((lxb_char_t *) "&&", 2, ctx);

        case LXB_GRAMMAR_TOKEN_LEFT_BRACKET:
            return func((lxb_char_t *) "[", 1, ctx);

        case LXB_GRAMMAR_TOKEN_RIGHT_BRACKET:
            return func((lxb_char_t *) "]", 1, ctx);

        case LXB_GRAMMAR_TOKEN_END_OF_FILE:
            return func((lxb_char_t *) "END_OF_FILE", 11, ctx);

        default:
            return func((lxb_char_t *) "UNDEFINED", 9, ctx);
    }

    return LXB_STATUS_OK;
}

const lxb_char_t *
lxb_grammar_token_name(lxb_grammar_token_t *token, size_t *len)
{
#define lxb_grammar_token_name_str(name)                                       \
    do {                                                                       \
        if (len != NULL) {                                                     \
            *len = strlen(name);                                               \
        }                                                                      \
                                                                               \
        return (lxb_char_t *) name;                                            \
    }                                                                          \
    while (0)

    switch (token->type) {
        case LXB_GRAMMAR_TOKEN_WHITESPACE:
            lxb_grammar_token_name_str("WHITESPACE");

        case LXB_GRAMMAR_TOKEN_UNQUOTED:
            lxb_grammar_token_name_str("UNQUOTED");

        case LXB_GRAMMAR_TOKEN_DELIM:
            lxb_grammar_token_name_str("DELIM");

        case LXB_GRAMMAR_TOKEN_ELEMENT:
            lxb_grammar_token_name_str("ELEMENT");

        case LXB_GRAMMAR_TOKEN_EQUALS:
            lxb_grammar_token_name_str("EQUALS");

        case LXB_GRAMMAR_TOKEN_STRING:
            lxb_grammar_token_name_str("STRING");

        case LXB_GRAMMAR_TOKEN_NUMBER:
            lxb_grammar_token_name_str("NUMBER");

        case LXB_GRAMMAR_TOKEN_ASTERISK:
            lxb_grammar_token_name_str("ASTERISK");

        case LXB_GRAMMAR_TOKEN_PLUS:
            lxb_grammar_token_name_str("PLUS");

        case LXB_GRAMMAR_TOKEN_EXCLAMATION:
            lxb_grammar_token_name_str("EXCLAMATION");

        case LXB_GRAMMAR_TOKEN_QUESTION:
            lxb_grammar_token_name_str("QUESTION");

        case LXB_GRAMMAR_TOKEN_HASH:
            lxb_grammar_token_name_str("HASH");

        case LXB_GRAMMAR_TOKEN_EXCLUDE_WS:
            lxb_grammar_token_name_str("EXCLUDE_WS");

        case LXB_GRAMMAR_TOKEN_EXCLUDE_SORT:
            lxb_grammar_token_name_str("EXCLUDE_SORT");

        case LXB_GRAMMAR_TOKEN_COUNT:
            lxb_grammar_token_name_str("COUNT");

        case LXB_GRAMMAR_TOKEN_RANGE:
            lxb_grammar_token_name_str("RANGE");

        case LXB_GRAMMAR_TOKEN_BAR:
            lxb_grammar_token_name_str("BAR");

        case LXB_GRAMMAR_TOKEN_DOUBLE_BAR:
            lxb_grammar_token_name_str("DOUBLE_BAR");

        case LXB_GRAMMAR_TOKEN_AND:
            lxb_grammar_token_name_str("AND");

        case LXB_GRAMMAR_TOKEN_LEFT_BRACKET:
            lxb_grammar_token_name_str("LEFT_BRACKET");

        case LXB_GRAMMAR_TOKEN_RIGHT_BRACKET:
            lxb_grammar_token_name_str("RIGHT_BRACKET");

        case LXB_GRAMMAR_TOKEN_END_OF_FILE:
            lxb_grammar_token_name_str("END_OF_FILE");

        default:
            lxb_grammar_token_name_str("UNDEFINED");
    }

#undef lxb_grammar_token_name_str
}
