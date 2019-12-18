/*
 * Copyright (C) 2018-2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tokenizer/state.h"
#include "lexbor/html/tokenizer/state_comment.h"
#include "lexbor/html/tokenizer/state_doctype.h"

#define LEXBOR_STR_RES_ALPHANUMERIC_CHARACTER
#define LEXBOR_STR_RES_ALPHA_CHARACTER
#include "lexbor/core/str_res.h"

#define LXB_HTML_TOKENIZER_RES_ENTITIES_SBST
#include "lexbor/html/tokenizer/res.h"


static const lxb_char_t *
lxb_html_tokenizer_state_data(lxb_html_tokenizer_t *tkz,
                              const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_plaintext(lxb_html_tokenizer_t *tkz,
                                   const lxb_char_t *data,
                                   const lxb_char_t *end);

/* Tag */
static const lxb_char_t *
lxb_html_tokenizer_state_tag_open(lxb_html_tokenizer_t *tkz,
                                  const lxb_char_t *data,
                                  const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_end_tag_open(lxb_html_tokenizer_t *tkz,
                                      const lxb_char_t *data,
                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_tag_name(lxb_html_tokenizer_t *tkz,
                                  const lxb_char_t *data,
                                  const lxb_char_t *end);

/* Attribute */
static const lxb_char_t *
lxb_html_tokenizer_state_attribute_name(lxb_html_tokenizer_t *tkz,
                                        const lxb_char_t *data,
                                        const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_after_attribute_name(lxb_html_tokenizer_t *tkz,
                                              const lxb_char_t *data,
                                              const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_before_attribute_value(lxb_html_tokenizer_t *tkz,
                                                const lxb_char_t *data,
                                                const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_attribute_value_double_quoted(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_attribute_value_single_quoted(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_attribute_value_unquoted(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_after_attribute_value_quoted(lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_bogus_comment_before(lxb_html_tokenizer_t *tkz,
                                              const lxb_char_t *data,
                                              const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_bogus_comment(lxb_html_tokenizer_t *tkz,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end);

/* Markup declaration */
static const lxb_char_t *
lxb_html_tokenizer_state_markup_declaration_open(lxb_html_tokenizer_t *tkz,
                                                 const lxb_char_t *data,
                                                 const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_markup_declaration_comment(lxb_html_tokenizer_t *tkz,
                                                    const lxb_char_t *data,
                                                    const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_markup_declaration_doctype(lxb_html_tokenizer_t *tkz,
                                                    const lxb_char_t *data,
                                                    const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_markup_declaration_cdata(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end);

/* CDATA Section */
static const lxb_char_t *
lxb_html_tokenizer_state_cdata_section_before(lxb_html_tokenizer_t *tkz,
                                              const lxb_char_t *data,
                                              const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_cdata_section(lxb_html_tokenizer_t *tkz,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_cdata_section_bracket(lxb_html_tokenizer_t *tkz,
                                               const lxb_char_t *data,
                                               const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_cdata_section_end(lxb_html_tokenizer_t *tkz,
                                           const lxb_char_t *data,
                                           const lxb_char_t *end);


/*
 * Helper function. No in the specification. For 12.2.5.1 Data state
 */
const lxb_char_t *
lxb_html_tokenizer_state_data_before(lxb_html_tokenizer_t *tkz,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end)
{
    if (tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_set_begin(tkz, data);
    }

    /*
     * Text node init param sets before emit token.
     */

    tkz->state = lxb_html_tokenizer_state_data;

    return data;
}

/*
 * 12.2.5.1 Data state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_data(lxb_html_tokenizer_t *tkz,
                              const lxb_char_t *data, const lxb_char_t *end)
{
    while (data != end) {
        switch (*data) {
            /* U+0026 AMPERSAND (&) */
            /* Processing after create token */

            /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                lxb_html_tokenizer_state_token_set_end(tkz, data);

                tkz->state = lxb_html_tokenizer_state_tag_open;
                return (data + 1);

            /*
             * U+0000 NULL
             * EOF
             */
            case 0x00:
                if (tkz->is_eof) {
                    /* Emit TEXT node if not empty */
                    if (tkz->token->begin != NULL) {
                        lxb_html_tokenizer_state_token_set_end_oef(tkz);
                    }

                    if (tkz->token->begin != tkz->token->end) {
                        tkz->token->tag_id = LXB_TAG__TEXT;
                        tkz->token->type = LXB_HTML_TOKEN_TYPE_TEXT;

                        lxb_html_tokenizer_state_token_done_wo_check_m(tkz,end);
                    }

                    return end;
                }

                tkz->token->type |= LXB_HTML_TOKEN_TYPE_NULL;

                lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                             LXB_HTML_TOKENIZER_ERROR_UNNUCH);
                break;
        }

        data++;
    }

    return data;
}

/*
 * Helper function. No in the specification. For 12.2.5.5 PLAINTEXT state
 */
const lxb_char_t *
lxb_html_tokenizer_state_plaintext_before(lxb_html_tokenizer_t *tkz,
                                          const lxb_char_t *data,
                                          const lxb_char_t *end)
{
    if (tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_set_begin(tkz, data);
    }

    tkz->token->tag_id = LXB_TAG__TEXT;
    tkz->token->type = LXB_HTML_TOKEN_TYPE_DATA;

    tkz->state = lxb_html_tokenizer_state_plaintext;

    return data;
}

/*
 * 12.2.5.5 PLAINTEXT state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_plaintext(lxb_html_tokenizer_t *tkz,
                                   const lxb_char_t *data,
                                   const lxb_char_t *end)
{
    while (data != end) {
        /*
         * U+0000 NULL
         * EOF
         */
        if (*data == 0x00) {
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
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.6 Tag open state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_tag_open(lxb_html_tokenizer_t *tkz,
                                  const lxb_char_t *data, const lxb_char_t *end)
{
    /* ASCII alpha */
    if (lexbor_str_res_alpha_character[ *data ] != LEXBOR_STR_RES_SLIP) {
        tkz->state = lxb_html_tokenizer_state_tag_name;

        lxb_html_tokenizer_state_token_emit_text_not_empty_m(tkz, end);
        lxb_html_tokenizer_state_token_set_begin(tkz, data);

        return data;
    }

    /* U+0021 EXCLAMATION MARK (!) */
    else if (*data == 0x21) {
        tkz->state = lxb_html_tokenizer_state_markup_declaration_open;

        lxb_html_tokenizer_state_token_emit_text_not_empty_m(tkz, end);

        return (data + 1);
    }

    /* U+002F SOLIDUS (/) */
    else if (*data == 0x2F) {
        tkz->state = lxb_html_tokenizer_state_end_tag_open;

        return (data + 1);
    }

    /* U+003F QUESTION MARK (?) */
    else if (*data == 0x3F) {
        tkz->state = lxb_html_tokenizer_state_bogus_comment_before;

        lxb_html_tokenizer_state_token_emit_text_not_empty_m(tkz, end);
        lxb_html_tokenizer_state_token_set_begin(tkz, data);

        lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_UNQUMAINOFTANA);

        return (data + 1);
    }

    /* EOF */
    else if (*data == 0x00) {
        if (tkz->is_eof) {
            /* Emit TEXT token */
            tkz->token->tag_id = LXB_TAG__TEXT;
            tkz->token->type = LXB_HTML_TOKEN_TYPE_TEXT;

            lxb_html_tokenizer_state_token_set_end_oef(tkz);

            lxb_html_tokenizer_error_add(tkz->parse_errors, tkz->token->end,
                                         LXB_HTML_TOKENIZER_ERROR_EOBETANA);

            lxb_html_tokenizer_state_token_emit_text_not_empty_m(tkz, end);

            return end;
        }
    }

    lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                 LXB_HTML_TOKENIZER_ERROR_INFICHOFTANA);

    tkz->state = lxb_html_tokenizer_state_data;

    return data;
}

/*
 * 12.2.5.7 End tag open state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_end_tag_open(lxb_html_tokenizer_t *tkz,
                                      const lxb_char_t *data,
                                      const lxb_char_t *end)
{
    /* ASCII alpha */
    if (lexbor_str_res_alpha_character[ *data ] != LEXBOR_STR_RES_SLIP) {
        tkz->state = lxb_html_tokenizer_state_tag_name;

        lxb_html_tokenizer_state_token_emit_text_not_empty_m(tkz, end);
        lxb_html_tokenizer_state_token_set_begin(tkz, data);

        tkz->token->type |= LXB_HTML_TOKEN_TYPE_CLOSE;

        return data;
    }

    /* U+003E GREATER-THAN SIGN (>) */
    else if (*data == 0x3E) {
        tkz->state = lxb_html_tokenizer_state_data;

        lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_MIENTANA);

        return (data + 1);
    }

    /* Fake EOF */
    else if (*data == 0x00) {
        if (tkz->is_eof) {
            /* Emit TEXT node */
            tkz->token->tag_id = LXB_TAG__TEXT;
            tkz->token->type = LXB_HTML_TOKEN_TYPE_TEXT;

            lxb_html_tokenizer_state_token_set_end_oef(tkz);

            lxb_html_tokenizer_error_add(tkz->parse_errors, tkz->token->end,
                                         LXB_HTML_TOKENIZER_ERROR_EOBETANA);

            lxb_html_tokenizer_state_token_emit_text_not_empty_m(tkz, end);

            return end;
        }
    }

    tkz->state = lxb_html_tokenizer_state_bogus_comment_before;

    lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                 LXB_HTML_TOKENIZER_ERROR_INFICHOFTANA);

    lxb_html_tokenizer_state_token_emit_text_not_empty_m(tkz, end);
    lxb_html_tokenizer_state_token_set_begin(tkz, data);

    return data;
}

/*
 * 12.2.5.8 Tag name state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_tag_name(lxb_html_tokenizer_t *tkz,
                                  const lxb_char_t *data, const lxb_char_t *end)
{
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
                lxb_html_tokenizer_state_token_set_end(tkz, data);

                tkz->state = lxb_html_tokenizer_state_before_attribute_name;
                return (data + 1);

            /* U+002F SOLIDUS (/) */
            case 0x2F:
                lxb_html_tokenizer_state_token_set_end(tkz, data);

                tkz->state = lxb_html_tokenizer_state_self_closing_start_tag;
                return (data + 1);

            /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                tkz->state = lxb_html_tokenizer_state_data_before;

                lxb_html_tokenizer_state_token_set_end(tkz, data);
                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return (data + 1);

            /* U+0000 NULL */
            case 0x00:
                if (tkz->is_eof) {
                    lxb_html_tokenizer_state_token_set_end_oef(tkz);

                    lxb_html_tokenizer_error_add(tkz->parse_errors,
                                               tkz->token->end,
                                               LXB_HTML_TOKENIZER_ERROR_EOINTA);
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
 * 12.2.5.32 Before attribute name state
 */
const lxb_char_t *
lxb_html_tokenizer_state_before_attribute_name(lxb_html_tokenizer_t *tkz,
                                               const lxb_char_t *data,
                                               const lxb_char_t *end)
{
    lxb_html_token_attr_t *attr;

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
                break;

            /* U+003D EQUALS SIGN (=) */
            case 0x3D:
                lxb_html_tokenizer_state_token_attr_add_m(tkz, attr, end);
                lxb_html_tokenizer_state_token_attr_set_name_begin(tkz, data);

                lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_HTML_TOKENIZER_ERROR_UNEQSIBEATNA);

                tkz->state = lxb_html_tokenizer_state_attribute_name;
                return (data + 1);

            /*
             * U+002F SOLIDUS (/)
             * U+003E GREATER-THAN SIGN (>)
             */
            case 0x2F:
            case 0x3E:
                tkz->state = lxb_html_tokenizer_state_after_attribute_name;
                return data;

            /* EOF */
            case 0x00:
                if (tkz->is_eof) {
                    tkz->state = lxb_html_tokenizer_state_after_attribute_name;
                    return data;
                }
                /* fall through */

            /* Anything else */
            default:
                lxb_html_tokenizer_state_token_attr_add_m(tkz, attr, end);
                lxb_html_tokenizer_state_token_attr_set_name_begin(tkz, data);

                tkz->state = lxb_html_tokenizer_state_attribute_name;
                return data;
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.33 Attribute name state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_attribute_name(lxb_html_tokenizer_t *tkz,
                                        const lxb_char_t *data,
                                        const lxb_char_t *end)
{
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
                lxb_html_tokenizer_state_token_attr_set_name_end(tkz, data);

                tkz->state = lxb_html_tokenizer_state_after_attribute_name;
                return data;

            /*
             * U+0000 NULL
             * EOF
             */
            case 0x00:
                if (tkz->is_eof) {
                    lxb_html_tokenizer_state_token_attr_set_name_end_oef(tkz);

                    tkz->state = lxb_html_tokenizer_state_after_attribute_name;
                    return data;
                }

                lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                             LXB_HTML_TOKENIZER_ERROR_UNNUCH);

                tkz->token->attr_last->type
                    |= LXB_HTML_TOKEN_ATTR_TYPE_NAME_NULL;

                break;

            /* U+003D EQUALS SIGN (=) */
            case 0x3D:
                lxb_html_tokenizer_state_token_attr_set_name_end(tkz, data);

                tkz->state = lxb_html_tokenizer_state_before_attribute_value;
                return (data + 1);

            /*
             * U+0022 QUOTATION MARK (")
             * U+0027 APOSTROPHE (')
             * U+003C LESS-THAN SIGN (<)
             */
            case 0x22:
            case 0x27:
            case 0x3C:
                lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                           LXB_HTML_TOKENIZER_ERROR_UNCHINATNA);
                break;

            default:
                break;
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.34 After attribute name state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_after_attribute_name(lxb_html_tokenizer_t *tkz,
                                              const lxb_char_t *data,
                                              const lxb_char_t *end)
{
    lxb_html_token_attr_t *attr;

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
                break;

            /* U+002F SOLIDUS (/) */
            case 0x2F:
                tkz->state = lxb_html_tokenizer_state_self_closing_start_tag;
                return (data + 1);

            /* U+003D EQUALS SIGN (=) */
            case 0x3D:
                tkz->state = lxb_html_tokenizer_state_before_attribute_value;
                return (data + 1);

            /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                tkz->state = lxb_html_tokenizer_state_data_before;

                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return (data + 1);

            case 0x00:
                if (tkz->is_eof) {
                    lxb_html_tokenizer_error_add(tkz->parse_errors,
                                               tkz->incoming_node->end,
                                               LXB_HTML_TOKENIZER_ERROR_EOINTA);
                    return end;
                }
                /* fall through */

            default:
                lxb_html_tokenizer_state_token_attr_add_m(tkz, attr, end);
                lxb_html_tokenizer_state_token_attr_set_name_begin(tkz, data);

                tkz->state = lxb_html_tokenizer_state_attribute_name;
                return data;
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.35 Before attribute value state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_before_attribute_value(lxb_html_tokenizer_t *tkz,
                                                const lxb_char_t *data,
                                                const lxb_char_t *end)
{
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
                break;

            /* U+0022 QUOTATION MARK (") */
            case 0x22:
                tkz->state =
                    lxb_html_tokenizer_state_attribute_value_double_quoted;

                return (data + 1);

            /* U+0027 APOSTROPHE (') */
            case 0x27:
                tkz->state =
                    lxb_html_tokenizer_state_attribute_value_single_quoted;

                return (data + 1);

            /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                tkz->state = lxb_html_tokenizer_state_data_before;

                lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                             LXB_HTML_TOKENIZER_ERROR_MIATVA);

                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return (data + 1);

            default:
                tkz->state = lxb_html_tokenizer_state_attribute_value_unquoted;
                return data;
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.36 Attribute value (double-quoted) state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_attribute_value_double_quoted(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    if (tkz->token->attr_last->value_begin == NULL && tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_attr_set_value_begin(tkz, data);
    }

    while (data != end) {
        switch (*data) {
            /* U+0022 QUOTATION MARK (") */
            case 0x22:
                lxb_html_tokenizer_state_token_attr_set_value_end(tkz, data);

                tkz->state =
                    lxb_html_tokenizer_state_after_attribute_value_quoted;

                return (data + 1);

            /* U+0026 AMPERSAND (&) */
            /* Processing after create token */
            /*case 0x26:
                tkz->state_return =
                    lxb_html_tokenizer_state_attribute_value_double_quoted;
                tkz->state = lxb_html_tokenizer_state_character_reference;
                return (data + 1);

                break;
             */

            /*
             * U+0000 NULL
             * EOF
             */
            case 0x00:
                if (tkz->is_eof) {
                    if (tkz->token->attr_last->value_begin != NULL) {
                     lxb_html_tokenizer_state_token_attr_set_value_end_oef(tkz);
                    }

                    lxb_html_tokenizer_error_add(tkz->parse_errors,
                                               tkz->incoming_node->end,
                                               LXB_HTML_TOKENIZER_ERROR_EOINTA);
                    return end;
                }

                tkz->token->attr_last->type
                    |= LXB_HTML_TOKEN_ATTR_TYPE_VALUE_NULL;

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
 * 12.2.5.37 Attribute value (single-quoted) state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_attribute_value_single_quoted(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    if (tkz->token->attr_last->value_begin == NULL && tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_attr_set_value_begin(tkz, data);
    }

    while (data != end) {
        switch (*data) {
            /* U+0027 APOSTROPHE (') */
            case 0x27:
                lxb_html_tokenizer_state_token_attr_set_value_end(tkz, data);

                tkz->state =
                    lxb_html_tokenizer_state_after_attribute_value_quoted;

                return (data + 1);

            /* U+0026 AMPERSAND (&) */
            /* Processing after create token */
            /*case 0x26:
                tkz->state_return =
                    lxb_html_tokenizer_state_attribute_value_single_quoted;
                tkz->state = lxb_html_tokenizer_state_character_reference;
                return (data + 1);
                break;
            */

            /*
             * U+0000 NULL
             * EOF
             */
            case 0x00:
                if (tkz->is_eof) {
                    if (tkz->token->attr_last->value_begin != NULL) {
                     lxb_html_tokenizer_state_token_attr_set_value_end_oef(tkz);
                    }

                    lxb_html_tokenizer_error_add(tkz->parse_errors,
                                               tkz->incoming_node->end,
                                               LXB_HTML_TOKENIZER_ERROR_EOINTA);
                    return end;
                }

                tkz->token->attr_last->type
                    |= LXB_HTML_TOKEN_ATTR_TYPE_VALUE_NULL;

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
 * 12.2.5.38 Attribute value (unquoted) state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_attribute_value_unquoted(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end)
{
    if (tkz->token->attr_last->value_begin == NULL && tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_attr_set_value_begin(tkz, data);
    }

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
                lxb_html_tokenizer_state_token_attr_set_value_end(tkz, data);

                tkz->state = lxb_html_tokenizer_state_before_attribute_name;
                return (data + 1);

            /* U+0026 AMPERSAND (&) */
            /* Processing after create token */
            /*case 0x26:
                tkz->state_return =
                    lxb_html_tokenizer_state_attribute_value_unquoted;
                tkz->state = lxb_html_tokenizer_state_character_reference;
                return (data + 1);

                break;
             */

            /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                tkz->state = lxb_html_tokenizer_state_data_before;

                lxb_html_tokenizer_state_token_attr_set_value_end(tkz, data);
                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return (data + 1);

            /*
             * U+0000 NULL
             * EOF
             */
            case 0x00:
                if (tkz->is_eof) {
                    if (tkz->token->attr_last->value_begin != NULL) {
                     lxb_html_tokenizer_state_token_attr_set_value_end_oef(tkz);
                    }

                    lxb_html_tokenizer_error_add(tkz->parse_errors,
                                                 tkz->incoming_node->end,
                                                 LXB_HTML_TOKENIZER_ERROR_EOINTA);
                    return end;
                }

                tkz->token->attr_last->type
                    |= LXB_HTML_TOKEN_ATTR_TYPE_VALUE_NULL;

                lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                             LXB_HTML_TOKENIZER_ERROR_UNNUCH);
                break;

            /*
             * U+0022 QUOTATION MARK (")
             * U+0027 APOSTROPHE (')
             * U+003C LESS-THAN SIGN (<)
             * U+003D EQUALS SIGN (=)
             * U+0060 GRAVE ACCENT (`)
             */
            case 0x22:
            case 0x27:
            case 0x3C:
            case 0x3D:
            case 0x60:
                lxb_html_tokenizer_error_add(tkz->parse_errors, tkz->token->end,
                                             LXB_HTML_TOKENIZER_ERROR_UNCHINUNATVA);
                /* fall through */

            default:
                break;
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.39 After attribute value (quoted) state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_after_attribute_value_quoted(lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
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
            tkz->state = lxb_html_tokenizer_state_before_attribute_name;

            return (data + 1);

        /* U+002F SOLIDUS (/) */
        case 0x2F:
            tkz->state = lxb_html_tokenizer_state_self_closing_start_tag;

            return (data + 1);

        /* U+003E GREATER-THAN SIGN (>) */
        case 0x3E:
            tkz->state = lxb_html_tokenizer_state_data_before;

            lxb_html_tokenizer_state_token_done_m(tkz, end);

            return (data + 1);

        /* EOF */
        case 0x00:
            if (tkz->is_eof) {
                lxb_html_tokenizer_error_add(tkz->parse_errors,
                                             tkz->incoming_node->end,
                                             LXB_HTML_TOKENIZER_ERROR_EOINTA);
                return end;
            }
            /* fall through */

        default:
            lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_HTML_TOKENIZER_ERROR_MIWHBEAT);

            tkz->state = lxb_html_tokenizer_state_before_attribute_name;

            return data;
    }

    return data;
}

/*
 * 12.2.5.40 Self-closing start tag state
 */
const lxb_char_t *
lxb_html_tokenizer_state_self_closing_start_tag(lxb_html_tokenizer_t *tkz,
                                                const lxb_char_t *data,
                                                const lxb_char_t *end)
{
    switch (*data) {
        /* U+003E GREATER-THAN SIGN (>) */
        case 0x3E:
            tkz->state = lxb_html_tokenizer_state_data_before;
            tkz->token->type |= LXB_HTML_TOKEN_TYPE_CLOSE_SELF;

            lxb_html_tokenizer_state_token_done_m(tkz, end);

            return (data + 1);

        /* EOF */
        case 0x00:
            if (tkz->is_eof) {
                lxb_html_tokenizer_error_add(tkz->parse_errors, tkz->token->end,
                                             LXB_HTML_TOKENIZER_ERROR_EOINTA);
                return end;
            }
            /* fall through */

        default:
            lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_HTML_TOKENIZER_ERROR_UNSOINTA);

            tkz->state = lxb_html_tokenizer_state_before_attribute_name;

            return data;
    }

    return data;
}

/*
 * Helper function. No in the specification. For 12.2.5.41 Bogus comment state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_bogus_comment_before(lxb_html_tokenizer_t *tkz,
                                              const lxb_char_t *data,
                                              const lxb_char_t *end)
{
    tkz->token->type = LXB_HTML_TOKEN_TYPE_DATA;
    tkz->token->tag_id = LXB_TAG__EM_COMMENT;

    tkz->state = lxb_html_tokenizer_state_bogus_comment;

    return data;
}

/*
 * 12.2.5.41 Bogus comment state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_bogus_comment(lxb_html_tokenizer_t *tkz,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end)
{
    while (data != end) {
        /* U+003E GREATER-THAN SIGN (>) */
        if (*data == 0x3E) {
            tkz->state = lxb_html_tokenizer_state_data_before;

            lxb_html_tokenizer_state_token_set_end(tkz, data);
            lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

            return (data + 1);
        }
        /*
         * EOF
         * U+0000 NULL
         */
        else if (*data == 0x00) {
            if (tkz->is_eof) {
                if (tkz->token->begin != NULL) {
                    lxb_html_tokenizer_state_token_set_end_oef(tkz);
                }

                lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

                return end;
            }

            lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                         LXB_HTML_TOKENIZER_ERROR_UNNUCH);

            tkz->token->type |= LXB_HTML_TOKEN_TYPE_NULL;
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.42 Markup declaration open state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_markup_declaration_open(lxb_html_tokenizer_t *tkz,
                                                 const lxb_char_t *data,
                                                 const lxb_char_t *end)
{
    /* Check first char for change parse state */
    if (tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_set_begin(tkz, data);
    }

    /* U+002D HYPHEN-MINUS characters (-) */
    if (*data == 0x2D) {
        if ((end - data) < 2) {
            tkz->state = lxb_html_tokenizer_state_markup_declaration_comment;
            return (data + 1);
        }

        if (data[1] == 0x2D) {
            tkz->state = lxb_html_tokenizer_state_comment_before_start;
            return (data + 2);
        }
    }
    /*
     * ASCII case-insensitive match for the word "DOCTYPE"
     * U+0044 character (D) or U+0064 character (d)
     */
    else if (*data == 0x44 || *data == 0x64) {
        if ((end - data) < 7) {
            tkz->markup = (lxb_char_t *) "doctype";

            tkz->state = lxb_html_tokenizer_state_markup_declaration_doctype;
            return data;
        }

        if (lexbor_str_data_ncasecmp((lxb_char_t *) "doctype", data, 7)) {
            tkz->state = lxb_html_tokenizer_state_doctype_before;
            return (data + 7);
        }
    }
    /* Case-sensitive match for the string "[CDATA["
     * (the five uppercase letters "CDATA" with a U+005B LEFT SQUARE BRACKET
     * character before and after)
     */
    else if (*data == 0x5B) {
        if ((end - data) < 7) {
            tkz->markup = (lxb_char_t *) "[CDATA[";

            tkz->state = lxb_html_tokenizer_state_markup_declaration_cdata;
            return data;
        }

        if (lexbor_str_data_ncmp((lxb_char_t *) "[CDATA[", data, 7)) {
            lxb_ns_id_t ns = lxb_html_tokenizer_current_namespace(tkz);

            if (ns != LXB_NS_HTML && ns != LXB_NS__UNDEF) {
                data += 7;

                lxb_html_tokenizer_state_token_set_begin(tkz, data);

                tkz->state = lxb_html_tokenizer_state_cdata_section_before;

                return data;
            }

            tkz->state = lxb_html_tokenizer_state_bogus_comment_before;

            return data;
        }
    }

    if (tkz->is_eof) {
        lxb_html_tokenizer_state_token_set_end_oef(tkz);

        tkz->token->begin = tkz->token->end;
        tkz->token->in_begin = tkz->incoming_node;
    }

    lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                 LXB_HTML_TOKENIZER_ERROR_INOPCO);

    tkz->state = lxb_html_tokenizer_state_bogus_comment_before;

    return data;
}

/*
 * Helper function. No in the specification. For 12.2.5.42
 * For a comment tag <!--
 */
static const lxb_char_t *
lxb_html_tokenizer_state_markup_declaration_comment(lxb_html_tokenizer_t *tkz,
                                                    const lxb_char_t *data,
                                                    const lxb_char_t *end)
{
    /* U+002D HYPHEN-MINUS characters (-) */
    if (*data == 0x2D) {
        tkz->state = lxb_html_tokenizer_state_comment_before_start;
        return (data + 1);
    }

    lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                 LXB_HTML_TOKENIZER_ERROR_INOPCO);

    tkz->state = lxb_html_tokenizer_state_bogus_comment_before;
    return data;
}

/*
 * Helper function. No in the specification. For 12.2.5.42
 * For a DOCTYPE tag <!DOCTYPE
 */
static const lxb_char_t *
lxb_html_tokenizer_state_markup_declaration_doctype(lxb_html_tokenizer_t *tkz,
                                                    const lxb_char_t *data,
                                                    const lxb_char_t *end)
{
    const lxb_char_t *pos;
    pos = lexbor_str_data_ncasecmp_first(tkz->markup, data, (end - data));

    if (pos == NULL) {
        lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_INOPCO);

        tkz->state = lxb_html_tokenizer_state_bogus_comment_before;
        return data;
    }

    if (*pos == '\0') {
        data = (data + (pos - tkz->markup));

        tkz->state = lxb_html_tokenizer_state_doctype_before;
        return data;
    }

    tkz->markup = pos;

    return end;
}

/*
 * Helper function. No in the specification. For 12.2.5.42
 * For a CDATA tag <![CDATA[
 */
static const lxb_char_t *
lxb_html_tokenizer_state_markup_declaration_cdata(lxb_html_tokenizer_t *tkz,
                                                  const lxb_char_t *data,
                                                  const lxb_char_t *end)
{
    const lxb_char_t *pos;
    pos = lexbor_str_data_ncasecmp_first(tkz->markup, data, (end - data));

    if (pos == NULL) {
        lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_INOPCO);

        tkz->state = lxb_html_tokenizer_state_bogus_comment_before;
        return data;
    }

    if (*pos == '\0') {
        lxb_ns_id_t ns = lxb_html_tokenizer_current_namespace(tkz);

        if (ns != LXB_NS_HTML && ns != LXB_NS__UNDEF) {
            data = (data + (pos - tkz->markup));

            tkz->state = lxb_html_tokenizer_state_cdata_section_before;
            return data;
        }

        tkz->state = lxb_html_tokenizer_state_bogus_comment_before;
        return data;
    }

    tkz->markup = pos;

    return end;
}

/*
 * Helper function. No in the specification. For 12.2.5.69
 */
static const lxb_char_t *
lxb_html_tokenizer_state_cdata_section_before(lxb_html_tokenizer_t *tkz,
                                              const lxb_char_t *data,
                                              const lxb_char_t *end)
{
    if (tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_set_begin(tkz, data);
    }
    else {
        lxb_html_tokenizer_state_token_set_begin(tkz, tkz->incoming_node->end);
    }

    tkz->token->tag_id = LXB_TAG__TEXT;
    tkz->token->type = LXB_HTML_TOKEN_TYPE_CDATA;

    tkz->state = lxb_html_tokenizer_state_cdata_section;

    return data;
}

/*
 * 12.2.5.69 CDATA section state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_cdata_section(lxb_html_tokenizer_t *tkz,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end)
{
    while (data != end) {
        /* U+005D RIGHT SQUARE BRACKET (]) */
        if (*data == 0x5D) {
            tkz->state = lxb_html_tokenizer_state_cdata_section_bracket;
            return (data + 1);
        }
        /* EOF */
        else if (*data == 0x00) {
            if (tkz->is_eof) {
                lxb_html_tokenizer_error_add(tkz->parse_errors,
                                             tkz->incoming_node->end,
                                             LXB_HTML_TOKENIZER_ERROR_EOINCD);

                if (tkz->token->begin != NULL) {
                    lxb_html_tokenizer_state_token_set_end_oef(tkz);
                }

                lxb_html_tokenizer_state_token_done_m(tkz, end);

                return end;
            }

            tkz->token->type |= LXB_HTML_TOKEN_TYPE_NULL;
        }

        data++;
    }

    return data;
}

/*
 * 12.2.5.70 CDATA section bracket state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_cdata_section_bracket(lxb_html_tokenizer_t *tkz,
                                               const lxb_char_t *data,
                                               const lxb_char_t *end)
{
    /* U+005D RIGHT SQUARE BRACKET (]) */
    if (*data == 0x5D) {
        tkz->state = lxb_html_tokenizer_state_cdata_section_end;
        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_cdata_section;
    return data;
}

/*
 * 12.2.5.71 CDATA section end state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_cdata_section_end(lxb_html_tokenizer_t *tkz,
                                           const lxb_char_t *data,
                                           const lxb_char_t *end)
{
    /* U+005D RIGHT SQUARE BRACKET (]) */
    if (*data == 0x5D) {
        return (data + 1);
    }
    /* U+003E GREATER-THAN SIGN character */
    else if (*data == 0x3E) {
        tkz->state = lxb_html_tokenizer_state_data_before;

        lxb_html_tokenizer_state_token_set_end_down(tkz, data, 2);
        lxb_html_tokenizer_state_token_done_m(tkz, end);

        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_cdata_section;

    return data;
}
