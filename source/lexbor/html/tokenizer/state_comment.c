/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tokenizer/state_comment.h"
#include "lexbor/html/tokenizer/state.h"


static const lxb_char_t *
lxb_html_tokenizer_state_comment_start(lxb_html_tokenizer_t *tkz,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_comment_start_dash(lxb_html_tokenizer_t *tkz,
                                            const lxb_char_t *data,
                                            const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_comment(lxb_html_tokenizer_t *tkz,
                                 const lxb_char_t *data,
                                 const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_comment_less_than_sign(lxb_html_tokenizer_t *tkz,
                                                const lxb_char_t *data,
                                                const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_comment_less_than_sign_bang(lxb_html_tokenizer_t *tkz,
                                                     const lxb_char_t *data,
                                                     const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_comment_less_than_sign_bang_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_comment_less_than_sign_bang_dash_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_comment_end_dash(lxb_html_tokenizer_t *tkz,
                                          const lxb_char_t *data,
                                          const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_comment_end(lxb_html_tokenizer_t *tkz,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end);

static const lxb_char_t *
lxb_html_tokenizer_state_comment_end_bang(lxb_html_tokenizer_t *tkz,
                                          const lxb_char_t *data,
                                          const lxb_char_t *end);


/*
 * Helper function. No in the specification. For 12.2.5.43
 */
const lxb_char_t *
lxb_html_tokenizer_state_comment_before_start(lxb_html_tokenizer_t *tkz,
                                              const lxb_char_t *data,
                                              const lxb_char_t *end)
{
    if (tkz->is_eof == false) {
        lxb_html_tokenizer_state_token_set_begin(tkz, data);
        lxb_html_tokenizer_state_token_set_end(tkz, data);
    }

    tkz->token->tag_id = LXB_TAG__EM_COMMENT;
    tkz->token->type = LXB_HTML_TOKEN_TYPE_DATA;

    return lxb_html_tokenizer_state_comment_start(tkz, data, end);
}

/*
 * 12.2.5.43 Comment start state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment_start(lxb_html_tokenizer_t *tkz,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end)
{
    /* U+002D HYPHEN-MINUS (-) */
    if (*data == 0x2D) {
        data++;
        tkz->state = lxb_html_tokenizer_state_comment_start_dash;
    }
    /* U+003E GREATER-THAN SIGN (>) */
    else if (*data == 0x3E) {
        tkz->state = lxb_html_tokenizer_state_data_before;

        lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_ABCLOFEMCO);

        lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

        data++;
    }
    else {
        tkz->state = lxb_html_tokenizer_state_comment;
    }

    return data;
}

/*
 * 12.2.5.44 Comment start dash state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment_start_dash(lxb_html_tokenizer_t *tkz,
                                            const lxb_char_t *data,
                                            const lxb_char_t *end)
{
    /* U+002D HYPHEN-MINUS (-) */
    if (*data == 0x2D) {
        tkz->state = lxb_html_tokenizer_state_comment_end;

        return (data + 1);
    }
    /* U+003E GREATER-THAN SIGN (>) */
    else if (*data == 0x3E) {
        tkz->state = lxb_html_tokenizer_state_data_before;

        lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_ABCLOFEMCO);

        lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

        return (data + 1);
    }
    /* EOF */
    else if (*data == 0x00) {
        if (tkz->is_eof) {
            lxb_html_tokenizer_error_add(tkz->parse_errors,
                                         tkz->incoming_node->end,
                                         LXB_HTML_TOKENIZER_ERROR_EOINCO);

            lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

            return end;
        }
    }

    tkz->state = lxb_html_tokenizer_state_comment;

    return data;
}

/*
 * 12.2.5.45 Comment state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment(lxb_html_tokenizer_t *tkz,
                                 const lxb_char_t *data,
                                 const lxb_char_t *end)
{
    while (data != end) {
        /* U+003C LESS-THAN SIGN (<) */
        if (*data == 0x3C) {
            tkz->state = lxb_html_tokenizer_state_comment_less_than_sign;

            return (data + 1);
        }
        /* U+002D HYPHEN-MINUS (-) */
        else if (*data == 0x2D) {
            tkz->state = lxb_html_tokenizer_state_comment_end_dash;

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

                lxb_html_tokenizer_error_add(tkz->parse_errors, tkz->token->end,
                                             LXB_HTML_TOKENIZER_ERROR_EOINCO);

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
 * 12.2.5.46 Comment less-than sign state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment_less_than_sign(lxb_html_tokenizer_t *tkz,
                                                const lxb_char_t *data,
                                                const lxb_char_t *end)
{
    /* U+0021 EXCLAMATION MARK (!) */
    if (*data == 0x21) {
        tkz->state = lxb_html_tokenizer_state_comment_less_than_sign_bang;

        return (data + 1);
    }
    /* U+003C LESS-THAN SIGN (<) */
    else if (*data == 0x3C) {
        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_comment;

    return data;
}

/*
 * 12.2.5.47 Comment less-than sign bang state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment_less_than_sign_bang(lxb_html_tokenizer_t *tkz,
                                                     const lxb_char_t *data,
                                                     const lxb_char_t *end)
{
    /* U+002D HYPHEN-MINUS (-) */
    if (*data == 0x2D) {
        tkz->state = lxb_html_tokenizer_state_comment_less_than_sign_bang_dash;

        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_comment;

    return data;
}

/*
 * 12.2.5.48 Comment less-than sign bang dash state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment_less_than_sign_bang_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    /* U+002D HYPHEN-MINUS (-) */
    if (*data == 0x2D) {
        tkz->state =
            lxb_html_tokenizer_state_comment_less_than_sign_bang_dash_dash;

        return (data + 1);
    }

    tkz->state = lxb_html_tokenizer_state_comment_end_dash;

    return data;
}

/*
 * 12.2.5.49 Comment less-than sign bang dash dash state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment_less_than_sign_bang_dash_dash(
                                                      lxb_html_tokenizer_t *tkz,
                                                      const lxb_char_t *data,
                                                      const lxb_char_t *end)
{
    /* U+003E GREATER-THAN SIGN (>) */
    if (*data == 0x3E) {
        tkz->state = lxb_html_tokenizer_state_comment_end;

        return data;
    }
    /* EOF */
    else if (*data == 0x00) {
        if (tkz->is_eof) {
            tkz->state = lxb_html_tokenizer_state_comment_end;

            return data;
        }
    }

    lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                 LXB_HTML_TOKENIZER_ERROR_NECO);

    tkz->state = lxb_html_tokenizer_state_comment_end;

    return data;
}

/*
 * 12.2.5.50 Comment end dash state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment_end_dash(lxb_html_tokenizer_t *tkz,
                                          const lxb_char_t *data,
                                          const lxb_char_t *end)
{
    /* U+002D HYPHEN-MINUS (-) */
    if (*data == 0x2D) {
        tkz->state = lxb_html_tokenizer_state_comment_end;

        return (data + 1);
    }
    /* EOF */
    else if (*data == 0x00) {
        if (tkz->is_eof) {
            lxb_html_tokenizer_error_add(tkz->parse_errors,
                                         tkz->incoming_node->end,
                                         LXB_HTML_TOKENIZER_ERROR_EOINCO);

            lxb_html_tokenizer_state_token_set_end_down(tkz,
                                                    tkz->incoming_node->end, 1);
            lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

            return end;
        }
    }

    tkz->state = lxb_html_tokenizer_state_comment;

    return data;
}

/*
 * 12.2.5.51 Comment end state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment_end(lxb_html_tokenizer_t *tkz,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end)
{
    /* U+003E GREATER-THAN SIGN (>) */
    if (*data == 0x3E) {
        /* Skep two '-' characters in comment tag end "-->"
         * For <!----> or <!-----> ...
         */
        tkz->state = lxb_html_tokenizer_state_data_before;

        lxb_html_tokenizer_state_token_set_end_down(tkz, data, 2);
        lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

        return (data + 1);
    }
    /* U+0021 EXCLAMATION MARK (!) */
    else if (*data == 0x21) {
        tkz->state = lxb_html_tokenizer_state_comment_end_bang;

        return (data + 1);
    }
    /* U+002D HYPHEN-MINUS (-) */
    else if (*data == 0x2D) {
        return (data + 1);
    }
    /* EOF */
    else if (*data == 0x00) {
        if (tkz->is_eof) {
            lxb_html_tokenizer_error_add(tkz->parse_errors,
                                         tkz->incoming_node->end,
                                         LXB_HTML_TOKENIZER_ERROR_EOINCO);

            lxb_html_tokenizer_state_token_set_end_down(tkz,
                                                    tkz->incoming_node->end, 2);
            lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

            return end;
        }
    }

    tkz->state = lxb_html_tokenizer_state_comment;

    return data;
}

/*
 * 12.2.5.52 Comment end bang state
 */
static const lxb_char_t *
lxb_html_tokenizer_state_comment_end_bang(lxb_html_tokenizer_t *tkz,
                                          const lxb_char_t *data,
                                          const lxb_char_t *end)
{
    /* U+002D HYPHEN-MINUS (-) */
    if (*data == 0x2D) {
        tkz->state = lxb_html_tokenizer_state_comment_end_dash;

        return (data + 1);
    }
    /* U+003E GREATER-THAN SIGN (>) */
    else if (*data == 0x3E) {
        tkz->state = lxb_html_tokenizer_state_data_before;

        lxb_html_tokenizer_error_add(tkz->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_INCLCO);

        lxb_html_tokenizer_state_token_set_end_down(tkz, data, 3);
        lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

        return (data + 1);
    }
    /* EOF */
    else if (*data == 0x00) {
        if (tkz->is_eof) {
            lxb_html_tokenizer_error_add(tkz->parse_errors,
                                         tkz->incoming_node->end,
                                         LXB_HTML_TOKENIZER_ERROR_EOINCO);

            lxb_html_tokenizer_state_token_set_end_down(tkz,
                                                    tkz->incoming_node->end, 3);
            lxb_html_tokenizer_state_token_done_wo_check_m(tkz, end);

            return end;
        }
    }

    tkz->state = lxb_html_tokenizer_state_comment;

    return data;
}
