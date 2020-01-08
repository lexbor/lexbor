/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tokenizer/state_script.h"
#include "lexbor/html/tokenizer/state.h"
#include "lexbor/html/in.h"

#define LEXBOR_STR_RES_ALPHA_CHARACTER
#include "lexbor/core/str_res.h"


static const lxb_char_t *
lxb_html_tokenizer_state_script_data(lxb_html_tokenizer_t *tkz,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_less_than_sign(lxb_html_tokenizer_t *tkz,
                                                    const lxb_char_t *data,
                                                    const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_end_tag_open(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_end_tag_name(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escape_start(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escape_start_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped(lxb_html_tokenizer_t *tkz,
                                             const lxb_char_t *data,
                                             const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_dash(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_dash_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_less_than_sign(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_end_tag_open(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_end_tag_name(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escape_start(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped(lxb_html_tokenizer_t *tkz,
                                                    const lxb_char_t *data,
                                                    const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped_dash_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped_less_than_sign(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped_end_tag_open(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escape_end(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);


/*
 * Helper function. No in the specification. For 12.2.5.4 Script data state
 */
const lxb_char_t *
lxb_html_tokenizer_state_script_data_before(lxb_html_tokenizer_t *tkz,
                                            const lxb_char_t *data,
                                            const lxb_char_t *end)
{
    if (tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_set_begin(tkz, data);
    }

    tkz->token->tag_id = LXB_TAG__TEXT;
    tkz->token->type = LXB_HTML_TOKEN_TYPE_DATA;

    tkz->state = lxb_html_tokenizer_state_script_data;

    return data;
}

/*
 * 12.2.5.4 Script data state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data(lxb_html_tokenizer_t *tkz,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end)
{
    while (data != end) {
        switch (*data) {
            /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                lxb_html_tokenizer_state_token_set_end(tkz, data);

                tkz->state =
                    lxb_html_tokenizer_state_script_data_less_than_sign;

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
 * 12.2.5.15 Script data less-than sign state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_less_than_sign(lxb_html_tokenizer_t *tkz,
                                                    const lxb_char_t *data,
                                                    const lxb_char_t *end)
{
    switch (*data) {
        /* U+002F SOLIDUS (/) */
        case 0x2F:
            tkz->state = lxb_html_tokenizer_state_script_data_end_tag_open;

            return (data + 1);

        /* U+0021 EXCLAMATION MARK (!) */
        case 0x21:
            tkz->state = lxb_html_tokenizer_state_script_data_escape_start;

            return (data + 1);

        default:
            tkz->state = lxb_html_tokenizer_state_script_data;

            break;
    }

    return data;
}

/*
 * 12.2.5.16 Script data end tag open state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_end_tag_open(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end)
{
    if (lexbor_str_res_alpha_character[*data] != LEXBOR_STR_RES_SLIP) {
        tkz->markup = data;
        tkz->tmp_incoming_node = tkz->incoming_node;

        tkz->state = lxb_html_tokenizer_state_script_data_end_tag_name;

        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_script_data;

    return data;
}

/*
 * 12.2.5.17 Script data end tag name state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_end_tag_name(lxb_html_tokenizer_t *tkz,
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

    tkz->state = lxb_html_tokenizer_state_script_data;

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

/*
 * 12.2.5.18 Script data escape start state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escape_start(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end)
{
    /* U+002D HYPHEN-MINUS (-) */
    if (*data == 0x2D) {
        tkz->state = lxb_html_tokenizer_state_script_data_escape_start_dash;

        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_script_data;

    return data;
}

/*
 * 12.2.5.19 Script data escape start dash state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escape_start_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    /* U+002D HYPHEN-MINUS (-) */
    if (*data == 0x2D) {
        tkz->state = lxb_html_tokenizer_state_script_data_escaped_dash_dash;

        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_script_data;

    return data;
}

/*
 * 12.2.5.20 Script data escaped state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped(lxb_html_tokenizer_t *tkz,
                                             const lxb_char_t *data,
                                             const lxb_char_t *end)
{
    while (data != end) {
        switch (*data) {
            /* U+002D HYPHEN-MINUS (-) */
            case 0x2D:
                tkz->state = lxb_html_tokenizer_state_script_data_escaped_dash;

                return (data + 1);

            /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                lxb_html_tokenizer_state_token_set_end(tkz, data);

                tkz->state =
                    lxb_html_tokenizer_state_script_data_escaped_less_than_sign;

                return (data + 1);

            /*
             * U+0000 NULL
             * EOF
             */
            case 0x00:
                if (tkz->is_eof) {
                    lxb_html_tokenizer_error_add(tkz->parse_errors,
                                       tkz->incoming_node->end,
                                       LXB_HTML_TOKENIZER_ERROR_EOINSCHTCOLITE);

                    lxb_html_tokenizer_state_token_set_end_oef(tkz);
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
 * 12.2.5.21 Script data escaped dash state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_dash(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end)
{
    switch (*data) {
        /* U+002D HYPHEN-MINUS (-) */
        case 0x2D:
            tkz->state = lxb_html_tokenizer_state_script_data_escaped_dash_dash;

            return (data + 1);

        /* U+003C LESS-THAN SIGN (<) */
        case 0x3C:
            lxb_html_tokenizer_state_token_set_end(tkz, data);

            tkz->state =
                lxb_html_tokenizer_state_script_data_escaped_less_than_sign;

            return (data + 1);

        /*
         * U+0000 NULL
         * EOF
         */
        case 0x00:
            if (tkz->is_eof) {
                lxb_html_tokenizer_error_add(tkz->parse_errors,
                                       tkz->incoming_node->end,
                                       LXB_HTML_TOKENIZER_ERROR_EOINSCHTCOLITE);

                lxb_html_tokenizer_state_token_set_end_oef(tkz);
                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return end;
            }

            lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_HTML_TOKENIZER_ERROR_UNNUCH);

            tkz->token->type |= LXB_HTML_TOKEN_TYPE_NULL;
            tkz->state = lxb_html_tokenizer_state_script_data_escaped;

            return (data + 1);

        default:
            tkz->state = lxb_html_tokenizer_state_script_data_escaped;

            return (data + 1);
    }

    return data;
}

/*
 * 12.2.5.22 Script data escaped dash dash state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_dash_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    switch (*data) {
        /* U+002D HYPHEN-MINUS (-) */
        case 0x2D:
            return (data + 1);

        /* U+003C LESS-THAN SIGN (<) */
        case 0x3C:
            lxb_html_tokenizer_state_token_set_end(tkz, data);

            tkz->state =
                lxb_html_tokenizer_state_script_data_escaped_less_than_sign;

            return (data + 1);

        /* U+003E GREATER-THAN SIGN (>) */
        case 0x3E:
            tkz->state = lxb_html_tokenizer_state_script_data;

            return (data + 1);

        /*
         * U+0000 NULL
         * EOF
         */
        case 0x00:
            if (tkz->is_eof) {
                lxb_html_tokenizer_error_add(tkz->parse_errors,
                                       tkz->incoming_node->end,
                                       LXB_HTML_TOKENIZER_ERROR_EOINSCHTCOLITE);

                lxb_html_tokenizer_state_token_set_end_oef(tkz);
                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return end;
            }

            lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_HTML_TOKENIZER_ERROR_UNNUCH);

            tkz->token->type |= LXB_HTML_TOKEN_TYPE_NULL;
            tkz->state = lxb_html_tokenizer_state_script_data_escaped;

            return (data + 1);

        default:
            tkz->state = lxb_html_tokenizer_state_script_data_escaped;

            return (data + 1);
    }

    return data;
}

/*
 * 12.2.5.23 Script data escaped less-than sign state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_less_than_sign(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    /* U+002F SOLIDUS (/) */
    if (*data == 0x2F) {
        tkz->state = lxb_html_tokenizer_state_script_data_escaped_end_tag_open;

        return (data + 1);
    }
    /* ASCII alpha */
    else if (lexbor_str_res_alpha_character[*data] != LEXBOR_STR_RES_SLIP) {
        tkz->markup = data;
        tkz->tmp_incoming_node = tkz->incoming_node;

        tkz->state = lxb_html_tokenizer_state_script_data_double_escape_start;

        return data;
    }

    tkz->state = lxb_html_tokenizer_state_script_data_escaped;

    return data;
}

/*
 * 12.2.5.24 Script data escaped end tag open state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_end_tag_open(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    if (lexbor_str_res_alpha_character[*data] != LEXBOR_STR_RES_SLIP) {
        tkz->markup = data;
        tkz->tmp_incoming_node = tkz->incoming_node;

        tkz->state = lxb_html_tokenizer_state_script_data_escaped_end_tag_name;

        return data;
    }

    tkz->state = lxb_html_tokenizer_state_script_data_escaped;

    return data;
}

/*
 * 12.2.5.25 Script data escaped end tag name state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_escaped_end_tag_name(
                                                      lxb_html_tokenizer_t *tkz,
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

    tkz->state = lxb_html_tokenizer_state_script_data_escaped;

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

/*
 * 12.2.5.26 Script data double escape start state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escape_start(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    bool is_cmp;

    while (data != end) {
        switch (*data) {
            /*
             * U+0009 CHARACTER TABULATION (tab)
             * U+000A LINE FEED (LF)
             * U+000C FORM FEED (FF)
             * U+000D CARRIAGE RETURN (CR)
             * U+0020 SPACE
             * U+002F SOLIDUS (/)
             * U+003E GREATER-THAN SIGN (>)
             */
            case 0x09:
            case 0x0A:
            case 0x0C:
            case 0x0D:
            case 0x20:
            case 0x2F:
            case 0x3E:
                is_cmp = lxb_html_in_ncasecmp(tkz->tmp_incoming_node,
                                              tkz->markup, data,
                                              (const lxb_char_t *) "script", 6);
                if (is_cmp) {
                    tkz->state =
                        lxb_html_tokenizer_state_script_data_double_escaped;

                    return (data + 1);
                }

                tkz->state = lxb_html_tokenizer_state_script_data_escaped;

                return (data + 1);

            default:
                if (lexbor_str_res_alpha_character[*data]
                    == LEXBOR_STR_RES_SLIP)
                {
                    tkz->state = lxb_html_tokenizer_state_script_data_escaped;

                    return data;
                }

                break;
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.27 Script data double escaped state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped(lxb_html_tokenizer_t *tkz,
                                                    const lxb_char_t *data,
                                                    const lxb_char_t *end)
{
    while (data != end) {
        switch (*data) {
            /* U+002D HYPHEN-MINUS (-) */
            case 0x2D:
                tkz->state =
                    lxb_html_tokenizer_state_script_data_double_escaped_dash;

                return (data + 1);

            /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                tkz->state =
             lxb_html_tokenizer_state_script_data_double_escaped_less_than_sign;

                return (data + 1);

            /*
             * U+0000 NULL
             * EOF
             */
            case 0x00:
                if (tkz->is_eof) {
                    lxb_html_tokenizer_error_add(tkz->parse_errors,
                                       tkz->incoming_node->end,
                                       LXB_HTML_TOKENIZER_ERROR_EOINSCHTCOLITE);

                    lxb_html_tokenizer_state_token_set_end_oef(tkz);
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
 * 12.2.5.28 Script data double escaped dash state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    switch (*data) {
        /* U+002D HYPHEN-MINUS (-) */
        case 0x2D:
            tkz->state =
                lxb_html_tokenizer_state_script_data_double_escaped_dash_dash;

            return (data + 1);

        /* U+003C LESS-THAN SIGN (<) */
        case 0x3C:
            tkz->state =
             lxb_html_tokenizer_state_script_data_double_escaped_less_than_sign;

            return (data + 1);

        /*
         * U+0000 NULL
         * EOF
         */
        case 0x00:
            if (tkz->is_eof) {
                lxb_html_tokenizer_error_add(tkz->parse_errors,
                                       tkz->incoming_node->end,
                                       LXB_HTML_TOKENIZER_ERROR_EOINSCHTCOLITE);

                lxb_html_tokenizer_state_token_set_end_oef(tkz);
                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return end;
            }

            lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_HTML_TOKENIZER_ERROR_UNNUCH);

            tkz->token->type |= LXB_HTML_TOKEN_TYPE_NULL;
            tkz->state = lxb_html_tokenizer_state_script_data_double_escaped;

            return (data + 1);

        default:
            tkz->state = lxb_html_tokenizer_state_script_data_double_escaped;

            return (data + 1);
    }

    return data;
}

/*
 * 12.2.5.29 Script data double escaped dash dash state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped_dash_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    switch (*data) {
        /* U+002D HYPHEN-MINUS (-) */
        case 0x2D:
            return (data + 1);

        /* U+003C LESS-THAN SIGN (<) */
        case 0x3C:
            tkz->state =
             lxb_html_tokenizer_state_script_data_double_escaped_less_than_sign;

            return (data + 1);

        /* U+003E GREATER-THAN SIGN (>) */
        case 0x3E:
            tkz->state = lxb_html_tokenizer_state_script_data;

            return (data + 1);

        /*
         * U+0000 NULL
         * EOF
         */
        case 0x00:
            if (tkz->is_eof) {
                lxb_html_tokenizer_error_add(tkz->parse_errors,
                                       tkz->incoming_node->end,
                                       LXB_HTML_TOKENIZER_ERROR_EOINSCHTCOLITE);

                lxb_html_tokenizer_state_token_set_end_oef(tkz);
                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return end;
            }

            lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_HTML_TOKENIZER_ERROR_UNNUCH);

            tkz->token->type |= LXB_HTML_TOKEN_TYPE_NULL;
            tkz->state = lxb_html_tokenizer_state_script_data_double_escaped;

            return (data + 1);

        default:
            tkz->state = lxb_html_tokenizer_state_script_data_double_escaped;

            return (data + 1);
    }

    return data;
}

/*
 * 12.2.5.30 Script data double escaped less-than sign state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped_less_than_sign(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    /* U+002F SOLIDUS (/) */
    if (*data == 0x2F) {
        tkz->state =
            lxb_html_tokenizer_state_script_data_double_escaped_end_tag_open;

        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_script_data_double_escaped;

    return data;
}

/*
 * 12.2.5.30.5 Helper function. No in the specification.
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escaped_end_tag_open(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    if (lexbor_str_res_alpha_character[*data] != LEXBOR_STR_RES_SLIP) {
        tkz->markup = data;
        tkz->tmp_incoming_node = tkz->incoming_node;

        tkz->state = lxb_html_tokenizer_state_script_data_double_escape_end;

        return data;
    }

    tkz->state = lxb_html_tokenizer_state_script_data_double_escaped;

    return data;
}

/*
 * 12.2.5.31 Script data double escape end state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_script_data_double_escape_end(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    bool is_cmp;

    while (data != end) {
        switch (*data) {
            /*
             * U+0009 CHARACTER TABULATION (tab)
             * U+000A LINE FEED (LF)
             * U+000C FORM FEED (FF)
             * U+000D CARRIAGE RETURN (CR)
             * U+0020 SPACE
             * U+002F SOLIDUS (/)
             * U+003E GREATER-THAN SIGN (>)
             */
            case 0x09:
            case 0x0A:
            case 0x0C:
            case 0x0D:
            case 0x20:
            case 0x2F:
            case 0x3E:
                is_cmp = lxb_html_in_ncasecmp(tkz->tmp_incoming_node,
                                              tkz->markup, data,
                                              (const lxb_char_t *) "script", 6);
                if (is_cmp) {
                    tkz->state =
                        lxb_html_tokenizer_state_script_data_escaped;

                    return (data + 1);
                }

                tkz->state =
                    lxb_html_tokenizer_state_script_data_double_escaped;

                return (data + 1);

            default:
                if (lexbor_str_res_alpha_character[*data]
                    == LEXBOR_STR_RES_SLIP)
                {
                    tkz->state =
                        lxb_html_tokenizer_state_script_data_double_escaped;

                    return data;
                }

                break;
        }

        data++;
    }

    return data;
}
