/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_TOKENIZER_STATE_H
#define LEXBOR_HTML_TOKENIZER_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/tokenizer.h"


#define lxb_html_tokenizer_state_token_set_begin(tkz, v_begin)                 \
    do {                                                                       \
        tkz->token->in_begin = tkz->incoming_node;                             \
        tkz->token->begin = v_begin;                                           \
    }                                                                          \
    while (0)

#define lxb_html_tokenizer_state_token_set_end(tkz, v_end)                     \
    tkz->token->end = v_end

#define lxb_html_tokenizer_state_token_set_end_down(tkz, v_end, offset)        \
    do {                                                                       \
        tkz->token->end = lexbor_in_node_pos_down(tkz->incoming_node, NULL,    \
                                                  v_end, offset);              \
    }                                                                          \
    while (0)

#define lxb_html_tokenizer_state_token_set_end_oef(tkz)                        \
    tkz->token->end = tkz->incoming_node->end

#define lxb_html_tokenizer_state_token_attr_add_m(tkz, attr, v_return)         \
    do {                                                                       \
        attr = lxb_html_token_attr_append(tkz->token, tkz->dobj_token_attr);   \
        if (attr == NULL) {                                                    \
            tkz->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;                  \
            return v_return;                                                   \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_html_tokenizer_state_token_attr_set_name_begin(tkz, v_begin)       \
    do {                                                                       \
        tkz->token->attr_last->in_name = tkz->incoming_node;                   \
        tkz->token->attr_last->name_begin = v_begin;                           \
    }                                                                          \
    while (0)

#define lxb_html_tokenizer_state_token_attr_set_name_end(tkz, v_end)           \
    tkz->token->attr_last->name_end = v_end

#define lxb_html_tokenizer_state_token_attr_set_name_end_oef(tkz)              \
    tkz->token->attr_last->name_end = tkz->incoming_node->end

#define lxb_html_tokenizer_state_token_attr_set_value_begin(tkz, v_begin)      \
    do {                                                                       \
        tkz->token->attr_last->in_value = tkz->incoming_node;                  \
        tkz->token->attr_last->value_begin = v_begin;                          \
    }                                                                          \
    while (0)

#define lxb_html_tokenizer_state_token_attr_set_value_end(tkz, v_end)          \
    tkz->token->attr_last->value_end = v_end

#define lxb_html_tokenizer_state_token_attr_set_value_end_oef(tkz)             \
    tkz->token->attr_last->value_end = tkz->incoming_node->end

#define _lxb_html_tokenizer_state_token_done_m(tkz, v_end)                     \
    tkz->token = tkz->callback_token_done(tkz, tkz->token,                     \
                                          tkz->callback_token_ctx);            \
    if (tkz->token == NULL) {                                                  \
        if (tkz->status == LXB_STATUS_OK) {                                    \
            tkz->status = LXB_STATUS_ERROR;                                    \
        }                                                                      \
        return v_end;                                                          \
    }

#define lxb_html_tokenizer_state_token_done_m(tkz, v_end)                      \
    do {                                                                       \
        if (tkz->token->begin != tkz->token->end) {                            \
            _lxb_html_tokenizer_state_token_done_m(tkz, v_end)                 \
        }                                                                      \
        lxb_html_token_clean(tkz->token);                                      \
    }                                                                          \
    while (0)

#define lxb_html_tokenizer_state_token_done_wo_check_m(tkz, v_end)             \
    do {                                                                       \
        _lxb_html_tokenizer_state_token_done_m(tkz, v_end)                     \
        lxb_html_token_clean(tkz->token);                                      \
    }                                                                          \
    while (0)

/* Emit TEXT node if not empty */
#define lxb_html_tokenizer_state_token_emit_text_not_empty_m(tkz, v_end)       \
    do {                                                                       \
        if (tkz->token->begin != tkz->token->end) {                            \
            tkz->token->tag_id = LXB_TAG__TEXT;                                \
            tkz->token->type = LXB_HTML_TOKEN_TYPE_TEXT;                       \
                                                                               \
            _lxb_html_tokenizer_state_token_done_m(tkz, v_end)                 \
            lxb_html_token_clean(tkz->token);                                  \
        }                                                                      \
    }                                                                          \
    while (0)


LXB_API const lxb_char_t *
lxb_html_tokenizer_state_data_before(lxb_html_tokenizer_t *tkz,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end);

LXB_API const lxb_char_t *
lxb_html_tokenizer_state_plaintext_before(lxb_html_tokenizer_t *tkz,
                                          const lxb_char_t *data,
                                          const lxb_char_t *end);

LXB_API const lxb_char_t *
lxb_html_tokenizer_state_before_attribute_name(lxb_html_tokenizer_t *tkz,
                                               const lxb_char_t *data,
                                               const lxb_char_t *end);

LXB_API const lxb_char_t *
lxb_html_tokenizer_state_self_closing_start_tag(lxb_html_tokenizer_t *tkz,
                                                const lxb_char_t *data,
                                                const lxb_char_t *end);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_TOKENIZER_STATE_H */
