/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tokenizer/state_rawtext.h"
#include "lexbor/html/tokenizer/state.h"
#include "lexbor/html/in.h"

#define LEXBOR_STR_RES_ALPHA_CHARACTER
#include "lexbor/core/str_res.h"


static const lxb_char_t *
lxb_html_tokenizer_state_rawtext(lxb_html_tokenizer_t *tkz,
                                const lxb_char_t *data,
                                const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_rawtext_less_than_sign(lxb_html_tokenizer_t *tkz,
                                               const lxb_char_t *data,
                                               const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_rawtext_end_tag_open(lxb_html_tokenizer_t *tkz,
                                             const lxb_char_t *data,
                                             const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_rawtext_end_tag_name(lxb_html_tokenizer_t *tkz,
                                             const lxb_char_t *data,
                                             const lxb_char_t *end);


/*
 * Helper function. No in the specification. For 12.2.5.3 RAWTEXT state
 */
const lxb_char_t *
lxb_html_tokenizer_state_rawtext_before(lxb_html_tokenizer_t *tkz,
                                        const lxb_char_t *data,
                                        const lxb_char_t *end)
{
    if (tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_set_begin(tkz, data);
    }

    tkz->token->tag_id = LXB_TAG__TEXT;
    tkz->token->type = LXB_HTML_TOKEN_TYPE_DATA;

    tkz->state = lxb_html_tokenizer_state_rawtext;

    return data;
}

/*
 * 12.2.5.3 RAWTEXT state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_rawtext(lxb_html_tokenizer_t *tkz,
                                 const lxb_char_t *data,
                                 const lxb_char_t *end)
{
    while (data != end) {
        switch (*data) {
            /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                lxb_html_tokenizer_state_token_set_end(tkz, data);

                tkz->state = lxb_html_tokenizer_state_rawtext_less_than_sign;

                return (data + 1);

            /*
             * U+0000 NULL
             * EOF
             */
            case 0x00:
                if (tkz->is_eof) {
                    if (tkz->token->begin != NULL) {
                        lxb_html_tokenizer_state_token_set_end_oef(tkz);
                    }

                    lxb_html_tokenizer_state_token_done_m(tkz, end);

                    return end;
                }

                tkz->token->type |= LXB_HTML_TOKEN_TYPE_NULL;

                lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                             LXB_HTML_TOKENIZER_ERROR_UNNUCH);
                break;

            default:
                break;
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.12 RAWTEXT less-than sign state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_rawtext_less_than_sign(lxb_html_tokenizer_t *tkz,
                                                const lxb_char_t *data,
                                                const lxb_char_t *end)
{
    /* U+002F SOLIDUS (/) */
    if (*data == 0x2F) {
        tkz->state = lxb_html_tokenizer_state_rawtext_end_tag_open;

        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_rawtext;

    return data;
}

/*
 * 12.2.5.13 RAWTEXT end tag open state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_rawtext_end_tag_open(lxb_html_tokenizer_t *tkz,
                                              const lxb_char_t *data,
                                              const lxb_char_t *end)
{
    if (lexbor_str_res_alpha_character[*data] != LEXBOR_STR_RES_SLIP) {
        tkz->markup = data;
        tkz->tmp_incoming_node = tkz->incoming_node;

        tkz->state = lxb_html_tokenizer_state_rawtext_end_tag_name;

        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_rawtext;

    return data;
}

/*
 * 12.2.5.14 RAWTEXT end tag name state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_rawtext_end_tag_name(lxb_html_tokenizer_t *tkz,
                                              const lxb_char_t *data,
                                              const lxb_char_t *end)
{
    const lxb_char_t *begin;
    lxb_tag_id_t tag_id;

    while (data != end) {
        switch (*data) {
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
                tag_id = lxb_html_in_tag_id(tkz->tmp_incoming_node,
                                   tkz->tags, tkz->markup, data, tkz->mraw);

                if (tkz->tmp_tag_id != tag_id) {
                    goto anything_else;
                }

                tkz->state = lxb_html_tokenizer_state_before_attribute_name;
                goto done;

            /* U+002F SOLIDUS (/) */
            case 0x2F:
                tag_id = lxb_html_in_tag_id(tkz->tmp_incoming_node,
                                   tkz->tags, tkz->markup, data, tkz->mraw);

                if (tkz->tmp_tag_id != tag_id) {
                    goto anything_else;
                }

                tkz->state = lxb_html_tokenizer_state_self_closing_start_tag;
                goto done;

            /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                tag_id = lxb_html_in_tag_id(tkz->tmp_incoming_node,
                                   tkz->tags, tkz->markup, data, tkz->mraw);

                if (tkz->tmp_tag_id != tag_id) {
                    goto anything_else;
                }

                tkz->state = lxb_html_tokenizer_state_data_before;

                /* Save begin position for close token */
                begin = tkz->markup;

                /* Emit text token */
                lxb_html_tokenizer_state_token_done_m(tkz, end);

                /* Init close token */
                tkz->token->begin = begin;
                tkz->token->end = data;
                tkz->token->in_begin = tkz->tmp_incoming_node;
                tkz->token->type |= LXB_HTML_TOKEN_TYPE_CLOSE;

                /* Emit close token */
                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return (data + 1);

            default:
                if (lexbor_str_res_alpha_character[*data]
                    == LEXBOR_STR_RES_SLIP)
                {
                    goto anything_else;
                }

                break;
        }

        data++;
    }

    return data;

anything_else:

    tkz->state = lxb_html_tokenizer_state_rawtext;

    return data;

done:

    /* Save begin position for close token */
    begin = tkz->markup;

    /* Emit text token */
    lxb_html_tokenizer_state_token_done_m(tkz, end);

    /* Init close token */
    tkz->token->begin = begin;
    tkz->token->end = data;
    tkz->token->in_begin = tkz->tmp_incoming_node;
    tkz->token->type |= LXB_HTML_TOKEN_TYPE_CLOSE;

    return (data + 1);
}
