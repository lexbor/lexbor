/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "unit/kv.h"

#define LEXBOR_STR_RES_ALPHANUMERIC_CHARACTER
#define LEXBOR_STR_RES_ALPHA_CHARACTER
#define LEXBOR_STR_RES_MAP_NUM
#define LEXBOR_STR_RES_MAP_HEX
#include "lexbor/core/str_res.h"


#define unit_kv_token_done_m(kv, data, end)                                      \
    do {                                                                         \
        kv->token = kv->rules(kv, kv->token, kv->rules_ctx);                     \
        if (kv->token == NULL) {                                                 \
            if (kv->status == LXB_STATUS_OK) {                                   \
                kv->status = LXB_STATUS_ERROR;                                   \
            }                                                                    \
                                                                                 \
            kv->error_pos = data;                                                \
                                                                                 \
            return end;                                                          \
        }                                                                        \
        memset(kv->token, 0, sizeof(unit_kv_token_t));                           \
    }                                                                            \
    while (0)


const lxb_char_t *
unit_kv_state_begin(unit_kv_t *kv, const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_before_data(unit_kv_t *kv,
                          const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_data_param(unit_kv_t *kv,
                         const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_data_param_before_count(unit_kv_t *kv, const lxb_char_t *data,
                                      const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_data_param_count(unit_kv_t *kv,
                               const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_before_data_body(unit_kv_t *kv,
                               const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_data_body_skip(unit_kv_t *kv,
                             const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_data_body(unit_kv_t *kv,
                        const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_data_body_before_end_skip(unit_kv_t *kv, const lxb_char_t *data,
                                        const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_data_body_before_end(unit_kv_t *kv,
                                   const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_data_body_end(unit_kv_t *kv,
                            const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_chars(unit_kv_t *kv, const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_string(unit_kv_t *kv,
                     const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_string_escape(unit_kv_t *kv,
                            const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_string_escape_u(unit_kv_t *kv,
                              const lxb_char_t *data, const lxb_char_t *end);

//static const lxb_char_t *
//unit_kv_state_number(unit_kv_t *kv,
//                     const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_number_minus(unit_kv_t *kv,
                           const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_number_null(unit_kv_t *kv,
                          const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_number_null_dot_digit(unit_kv_t *kv,
                                    const lxb_char_t *data,
                                    const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_number_null_dot_digit_e(unit_kv_t *kv,
                                      const lxb_char_t *data,
                                      const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_number_null_dot_digit_e_digit(unit_kv_t *kv,
                                            const lxb_char_t *data,
                                            const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_number_digit(unit_kv_t *kv,
                           const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_start_comment(unit_kv_t *kv,
                            const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_comment(unit_kv_t *kv,
                      const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
unit_kv_state_before_end_comment(unit_kv_t *kv,
                                 const lxb_char_t *data, const lxb_char_t *end);


const lxb_char_t *
unit_kv_state_begin(unit_kv_t *kv, const lxb_char_t *data, const lxb_char_t *end)
{
    switch (*data) {
        /*
         * U+0009 CHARACTER TABULATION (tab)
         * U+000C FORM FEED (FF)
         * U+0020 SPACE
         */
        case 0x09:
        case 0x0C:
        case 0x20:
            break;

        /* U+000A LINE FEED (LF) */
        case 0x0A:
            kv->line_count++;
            kv->line_begin = data;

            break;

        /* U+007B LEFT CURLY BRACKET ({) */
        case 0x7B:
            kv->token->pos.begin = data;
            kv->token->type = UNIT_KV_TOKEN_TYPE_LEFT_CURLY_BRACKET;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            unit_kv_token_done_m(kv, data, end);

            break;

        /* U+007D RIGHT CURLY BRACKET (}) */
        case 0x7D:
            kv->token->pos.begin = data;
            kv->token->type = UNIT_KV_TOKEN_TYPE_RIGHT_CURLY_BRACKET;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            unit_kv_token_done_m(kv, data, end);

            break;

        /* U+005B LEFT SQUARE BRACKET ([) */
        case 0x5B:
            kv->token->pos.begin = data;
            kv->token->type = UNIT_KV_TOKEN_TYPE_LEFT_SQUARE_BRACKET;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            unit_kv_token_done_m(kv, data, end);

            break;

        /* U+005D RIGHT SQUARE BRACKET (]) */
        case 0x5D:
            kv->token->pos.begin = data;
            kv->token->type = UNIT_KV_TOKEN_TYPE_RIGHT_SQUARE_BRACKET;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            unit_kv_token_done_m(kv, data, end);

            break;

        /* U+003A COLON (:) */
        case 0x3A:
            kv->token->pos.begin = data;
            kv->token->type = UNIT_KV_TOKEN_TYPE_COLON;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            unit_kv_token_done_m(kv, data, end);

            break;

        /* U+002C COMMA (,) */
        case 0x2C:
            kv->token->pos.begin = data;
            kv->token->type = UNIT_KV_TOKEN_TYPE_COMMA;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            unit_kv_token_done_m(kv, data, end);

            break;

        /* U+0022 QUOTATION MARK (") */
        case 0x22:
            kv->token->pos.begin = data;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            lexbor_str_init(&kv->token->value.str, kv->mraw, 1);
            kv->state = unit_kv_state_string;

            return (data + 1);

        /* U+0024 DOLLAR SIGN ($) */
        case 0x24:
            kv->token->pos.begin = data;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            lexbor_str_clean(&kv->var_name);

            kv->state = unit_kv_state_before_data;

            return (data + 1);

        /* U+002D HYPHEN-MINUS (-) */
        case 0x2D:
            kv->token->pos.begin = data;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            kv->state = unit_kv_state_number_minus;

            return data + 1;

        /* U+0030 (0) */
        case 0x30:
            kv->token->pos.begin = data;
            kv->token->pos.line_begin = kv->line_begin;
            kv->token->pos.line_count = kv->line_count;

            kv->num_negative = false;
            kv->state = unit_kv_state_number_null;

            return (data + 1);

        /* U+002F SOLIDUS (/) */
        case 0x2F:
            kv->state = unit_kv_state_start_comment;

            return (data + 1);

        /* EOF */
        case 0x00:
            if (kv->is_eof) {
                return end;
            }
            /* fall through */

        default:
            /* digit */
            if (lexbor_str_res_map_num[*data] != LEXBOR_STR_RES_SLIP) {
                kv->token->pos.begin = data;
                kv->token->pos.line_begin = kv->line_begin;
                kv->token->pos.line_count = kv->line_count;

                kv->num_negative = false;
                kv->num_digits = 0;

                kv->state = unit_kv_state_number_digit;

                return data;
            }

            if (lexbor_str_res_alpha_character[*data] != LEXBOR_STR_RES_SLIP) {
                kv->token->pos.begin = data;
                kv->token->pos.line_begin = kv->line_begin;
                kv->token->pos.line_count = kv->line_count;

                lexbor_str_init(&kv->token->value.str, kv->mraw, 1);

                kv->state = unit_kv_state_chars;

                return data;
            }

            kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
            kv->error_pos = data;

            return end;
    }

    return (data + 1);
}

static const lxb_char_t *
unit_kv_state_before_data(unit_kv_t *kv,
                          const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *begin = data;

    /* EOF */
    if (kv->is_eof) {
        kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
        kv->error_pos = data;

        return end;
    }

    while (data < end) {
        /* U+007B LEFT CURLY BRACKET ({) */
        if (*data == 0x7B) {
            kv->state = unit_kv_state_data_param;

            begin = lexbor_str_append(&kv->var_name, kv->mraw,
                                      begin, (data - begin));

            data++;

            goto append;
        }
        else if (lexbor_str_res_alphanumeric_character[*data] == LEXBOR_STR_RES_SLIP) {
            kv->state = unit_kv_state_before_data_body;

            begin = lexbor_str_append(&kv->var_name, kv->mraw,
                                      begin, (data - begin));
            goto append;
        }

        data++;
    }

append:

    if (begin == NULL) {
        kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        return end;
    }

    return data;
}

static const lxb_char_t *
unit_kv_state_data_param(unit_kv_t *kv,
                         const lxb_char_t *data, const lxb_char_t *end)
{
    kv->data_skip = *data;
    kv->state = unit_kv_state_data_param_before_count;

    return (data + 1);
}

static const lxb_char_t *
unit_kv_state_data_param_before_count(unit_kv_t *kv, const lxb_char_t *data,
                                      const lxb_char_t *end)
{
    /* U+002C COMMA (,) */
    if (*data != 0x2C) {
        kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
        kv->error_pos = data;

        return end;
    }

    kv->state = unit_kv_state_data_param_count;

    return (data + 1);
}

static const lxb_char_t *
unit_kv_state_data_param_count(unit_kv_t *kv,
                               const lxb_char_t *data, const lxb_char_t *end)
{
    /* U+007D RIGHT CURLY BRACKET (}) */
    if (*data == 0x7D) {
        kv->data_skip_count = 999999UL;
        kv->state = unit_kv_state_before_data_body;

        return (data + 1);
    }

    kv->data_skip_count = 0;

    while (data < end) {
        /* U+007D RIGHT CURLY BRACKET (}) */
        if (lexbor_str_res_map_num[*data] == LEXBOR_STR_RES_SLIP) {
            if (*data == 0x7D) {
                kv->state = unit_kv_state_before_data_body;

                return (data + 1);
            }

            break;
        }

        kv->data_skip_count = (*data - 0x30) + kv->data_skip_count * 10;

        data++;
    }

    kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
    kv->error_pos = data;

    return end;
}

static const lxb_char_t *
unit_kv_state_before_data_body(unit_kv_t *kv,
                               const lxb_char_t *data, const lxb_char_t *end)
{
    while (data < end) {
        switch (*data) {
            /*
             * U+0009 CHARACTER TABULATION (tab)
             * U+000C FORM FEED (FF)
             * U+0020 SPACE
             */
            case 0x09:
            case 0x0C:
            case 0x20:
                break;

            /* U+000A LINE FEED (LF) */
            case 0x0A:
                kv->line_count++;
                kv->line_begin = data;

                lxb_char_t *begin = lexbor_str_init(&kv->token->value.str,
                                                    kv->mraw, 1);
                if (begin == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                kv->state = unit_kv_state_data_body_skip;

                return (data + 1);

            /* EOF */
            case 0x00:
                if (kv->is_eof) {
                    kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
                    kv->error_pos = data;

                    return end;
                }
                /* fall through */

            default:
                kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
                kv->error_pos = data;

                return end;
        }

        data++;
    }

    return data;
}

static const lxb_char_t *
unit_kv_state_data_body_skip(unit_kv_t *kv,
                             const lxb_char_t *data, const lxb_char_t *end)
{
    size_t i = kv->data_skip_count;

    while (i != 0 && *data == kv->data_skip) {
        i--;
        data++;
    }

    kv->state = unit_kv_state_data_body;

    return data;
}

static const lxb_char_t *
unit_kv_state_data_body(unit_kv_t *kv,
                        const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *begin = data;

    while (data < end) {
        switch (*data) {
            /* U+000A LINE FEED (LF) */
            case 0x0A:
                begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                          begin, (data - begin));
                if (begin == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                kv->line_count++;
                kv->line_begin = data;

                kv->state = unit_kv_state_data_body_before_end_skip;

                return (data + 1);

            /* U+005C REVERSE SOLIDUS (\) */
            case 0x5C:
                begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                          begin, (data - begin));
                if (begin == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                kv->state = unit_kv_state_string_escape;
                kv->state_return = unit_kv_state_data_body;

                return (data + 1);

            /* U+0024 DOLLAR SIGN ($) */
            case 0x24:
                kv->state = unit_kv_state_data_body_end;

                return (data + 1);

            /* EOF */
            case 0x00:
                if (kv->is_eof) {
                    kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
                    kv->error_pos = data;

                    return end;
                }
                /* fall through */

            default:
                break;
        }

        data++;
    }

    begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                              begin, (data - begin));
    if (begin == NULL) {
        kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        return end;
    }

    return data;
}

static const lxb_char_t *
unit_kv_state_data_body_before_end_skip(unit_kv_t *kv, const lxb_char_t *data,
                                        const lxb_char_t *end)
{
    size_t i = kv->data_skip_count;

    while (i != 0 && *data == kv->data_skip) {
        i--;
        data++;
    }

    kv->state = unit_kv_state_data_body_before_end;

    return data;
}

static const lxb_char_t *
unit_kv_state_data_body_before_end(unit_kv_t *kv,
                                   const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *begin = data, *res;

    while (data < end) {
        switch (*data) {
            /*
             * U+0009 CHARACTER TABULATION (tab)
             * U+000C FORM FEED (FF)
             * U+0020 SPACE
             */
            case 0x09:
            case 0x0C:
            case 0x20:
                break;

            /* U+000A LINE FEED (LF) */
            case 0x0A:
                res = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                        (const lxb_char_t *) "\n", 1);
                if (res == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                          begin, (data - begin));
                if (begin == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                kv->line_count++;
                kv->line_begin = data;

                kv->state = unit_kv_state_data_body_before_end_skip;

                return (data + 1);

            /* U+0024 DOLLAR SIGN ($) */
            case 0x24:
                kv->state = unit_kv_state_data_body_end;

                return (data + 1);

            default:
                res = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                        (const lxb_char_t *) "\n", 1);
                if (res == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                          begin, (data - begin));
                if (begin == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                kv->state = unit_kv_state_data_body;

                return data;
        }

        data++;
    }

    return data;
}

static const lxb_char_t *
unit_kv_state_data_body_end(unit_kv_t *kv,
                            const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *res;
    const lxb_char_t *begin = data;

    while (data < end) {
        if (lexbor_str_res_alphanumeric_character[*data] == LEXBOR_STR_RES_SLIP)
        {
            if ((data - begin) != kv->var_name.length
                || lexbor_str_data_ncmp(kv->var_name.data, begin,
                                        kv->var_name.length) == false)
            {
                res = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                        (const lxb_char_t *) "\n$", 2);
                if (res == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                res = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                        begin, (data - begin));
                if (res == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                kv->state = unit_kv_state_data_body;
                return data;
            }

            kv->token->type = UNIT_KV_TOKEN_TYPE_DATA;
            unit_kv_token_done_m(kv, data, end);

            kv->state = unit_kv_state_begin;
            return data;
        }

        data++;
    }

    return data;
}


static const lxb_char_t *
unit_kv_state_chars(unit_kv_t *kv, const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *begin = data;

    while (data < end) {
        /*
         * Alphanumeric
         * U+002D HYPHEN-MINUS (-)
         * U+005F LOW LINE (_)
         */
        if (lexbor_str_res_alphanumeric_character[*data] == LEXBOR_STR_RES_SLIP
            && *data != 0x2D && *data != 0x5F)
        {
            begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                      begin, (data - begin));
            if (begin == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return end;
            }

            kv->token->type = UNIT_KV_TOKEN_TYPE_CHARS;
            unit_kv_token_done_m(kv, data, end);

            kv->state = unit_kv_state_begin;
            return data;
        }

        data++;
    }

    begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                              begin, (data - begin));
    if (begin == NULL) {
        kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        return end;
    }

    return data;
}

static const lxb_char_t *
unit_kv_state_string(unit_kv_t *kv, const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *begin = data;

    while (data < end) {
        switch (*data) {
            /* U+000A LINE FEED (LF) */
            case 0x0A:
                kv->line_count++;
                kv->line_begin = data;
                break;

            /* U+005C REVERSE SOLIDUS (\) */
            case 0x5C:
                begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                          begin, (data - begin));
                if (begin == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                kv->state = unit_kv_state_string_escape;
                kv->state_return = unit_kv_state_string;

                return (data + 1);

            /* U+0022 QUOTATION MARK (") */
            case 0x22:
                begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                                          begin, (data - begin));
                if (begin == NULL) {
                    kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    return end;
                }

                kv->token->type = UNIT_KV_TOKEN_TYPE_STRING;
                unit_kv_token_done_m(kv, data, end);

                kv->state = unit_kv_state_begin;
                return (data + 1);

            /* EOF */
            case 0x00:
                if (kv->is_eof) {
                    kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
                    kv->error_pos = data;

                    return end;
                }

            default:
                break;
        }

        data++;
    }

    begin = lexbor_str_append(&kv->token->value.str, kv->mraw,
                              begin, (data - begin));
    if (begin == NULL) {
        kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        return end;
    }

    return data;
}

static const lxb_char_t *
unit_kv_state_string_escape(unit_kv_t *kv,
                            const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *res;

    switch (*data) {
        /* U+0022 QUOTATION MARK (") */
        case 0x22:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x22);
            break;

        /* U+005C REVERSE SOLIDUS (\) */
        case 0x5C:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x5C);
            break;

        /* U+002F SOLIDUS (/) */
        case 0x2F:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x2F);
            break;

        /* U+0062 LATIN SMALL LETTER B (b) */
        case 0x62:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x08);
            break;

        /* U+0066 LATIN SMALL LETTER F (f) */
        case 0x66:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x0C);
            break;

        /* U+006E LATIN SMALL LETTER N (n) */
        case 0x6E:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x0A);
            break;

        /* U+0072 LATIN SMALL LETTER R (r) */
        case 0x72:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x0D);
            break;

        /* U+0074 LATIN SMALL LETTER T (t) */
        case 0x74:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x09);
            break;

        /* U+0075 LATIN SMALL LETTER U (u) */
        case 0x75:
            kv->state = unit_kv_state_string_escape_u;
            kv->count = 4;

            return (data + 1);

        /* U+0030 (0) */
        case 0x30:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x00);
            break;

        /* EOF */
        case 0x00:
            if (kv->is_eof) {
                kv->state = kv->state_return;
                return data;
            }
            /* fall through */

        default:
            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, 0x5C);
            if (res == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return end;
            }

            res = lexbor_str_append_one(&kv->token->value.str, kv->mraw, *data);

            /* U+000A LINE FEED (LF) */
            if (*data == 0x0A) {
                kv->line_count++;
                kv->line_begin = data;
            }

            break;
    }

    if (res == NULL) {
        kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        return end;
    }

    kv->state = kv->state_return;

    return (data + 1);
}

static const lxb_char_t *
unit_kv_state_string_escape_u(unit_kv_t *kv,
                              const lxb_char_t *data, const lxb_char_t *end)
{
    /* EOF */
    if (kv->is_eof && *data == 0x00) {
        kv->state = kv->state_return;
        return data;
    }

    while (data < end) {
        if (kv->count == 0) {
            lexbor_str_t *str = &kv->token->value.str;

            if (kv->num <= 0x007F) {
                lexbor_str_append_one(str, kv->mraw, kv->num);
            }
            else {
                /* 110xxxxx 10xxxxxx */
                lexbor_str_append_one(str, kv->mraw, (0xC0 | (kv->num >> 6  )));
                lexbor_str_append_one(str, kv->mraw, (0x80 | (kv->num & 0x3F)));
            }

            kv->state = kv->state_return;
            return data;
        }

        if (lexbor_str_res_map_hex[*data] == LEXBOR_STR_RES_SLIP) {
            kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
            kv->error_pos = data;

            return end;
        }

        kv->count--;

        kv->num <<= 4;
        kv->num |= lexbor_str_res_map_hex[ *data ];

        data++;
    }

    return data;
}

//static const lxb_char_t *
//unit_kv_state_number(unit_kv_t *kv,
//                     const lxb_char_t *data, const lxb_char_t *end)
//{
//    /* U+002D HYPHEN-MINUS (-) */
//    if (*data == 0x2D) {
//        kv->state = unit_kv_state_number_minus;
//        return (data + 1);
//    }
//    /* U+0030 (0) */
//    else if (*data == 0x30) {
//        kv->state = unit_kv_state_number_null;
//        return (data + 1);
//    }
//    else if (lexbor_str_res_map_num[*data] != LEXBOR_STR_RES_SLIP) {
//        kv->state = unit_kv_state_number_digit;
//        return data;
//    }
//
//    /* and EOF */
//    kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
//    kv->error_pos = data;
//
//    return end;
//}

static const lxb_char_t *
unit_kv_state_number_minus(unit_kv_t *kv,
                           const lxb_char_t *data, const lxb_char_t *end)
{
    kv->num_negative = true;

    /* U+0030 (0) */
    if (*data == 0x30) {
        kv->state = unit_kv_state_number_null;
        return (data + 1);
    }
    else if (lexbor_str_res_map_num[*data] != LEXBOR_STR_RES_SLIP) {
        kv->state = unit_kv_state_number_digit;
        return data;
    }

    /* and EOF */
    kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
    kv->error_pos = data;

    return end;
}

static const lxb_char_t *
unit_kv_state_number_null(unit_kv_t *kv,
                          const lxb_char_t *data, const lxb_char_t *end)
{
    /* U+002E FULL STOP (.) */
    if (*data == 0x2E) {
        kv->token->value.num.is_float = true;

        kv->num_digits = 0;
        kv->num_exponent = 0;

        kv->state = unit_kv_state_number_null_dot_digit;
        return (data + 1);
    }
    /*
     * U+0045 LATIN SMALL LETTER E (E)
     * U+0065 LATIN SMALL LETTER E (e)
     */
    else if (*data == 0x45 || *data == 0x65) {
        kv->token->value.num.is_float = false;

        kv->state = unit_kv_state_number_null_dot_digit_e;
        return (data + 1);
    }

    kv->token->value.num.is_float = false;
    kv->token->value.num.value.l = 0L;

    kv->token->type = UNIT_KV_TOKEN_TYPE_NUMBER;
    unit_kv_token_done_m(kv, data, end);

    /* and EOF */
    if (kv->is_eof && *data == 0x00) {
        return end;
    }

    kv->state = unit_kv_state_begin;

    return data;
}

static const lxb_char_t *
unit_kv_state_number_null_dot_digit(unit_kv_t *kv,
                                    const lxb_char_t *data, const lxb_char_t *end)
{
    while (data < end) {
        /*
         * U+0045 LATIN SMALL LETTER E (E)
         * U+0065 LATIN SMALL LETTER E (e)
         */
        if (*data == 0x45 || *data == 0x65) {
            if (kv->num_negative) {
                kv->num_digits = -kv->num_digits;
            }

            kv->state = unit_kv_state_number_null_dot_digit_e;
            return (data + 1);
        }
        else if (lexbor_str_res_map_num[*data] == LEXBOR_STR_RES_SLIP) {
            if (kv->num_negative) {
                kv->num_digits = -kv->num_digits;
            }

            kv->state = unit_kv_state_number_null_dot_digit_e_digit;
            return data;
        }

        kv->num_digits = (*data - 0x30) + kv->num_digits * 10;
        kv->num_exponent--;

        data++;
    }

    return data;
}

static const lxb_char_t *
unit_kv_state_number_null_dot_digit_e(unit_kv_t *kv,
                                      const lxb_char_t *data,
                                      const lxb_char_t *end)
{
    kv->num_decimals = 0;

    /* U+002B PLUS SIGN (+) */
    if (*data == 0x2B) {
        kv->num_negative = false;
        kv->state = unit_kv_state_number_null_dot_digit_e_digit;

        return (data + 1);
    }
    /* U+002D HYPHEN-MINUS (-) */
    else if(*data == 0x2D) {
        kv->num_negative = true;
        kv->state = unit_kv_state_number_null_dot_digit_e_digit;

        return (data + 1);
    }

    /* and EOF */
    kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
    kv->error_pos = data;

    return end;
}

static const lxb_char_t *
unit_kv_state_number_null_dot_digit_e_digit(unit_kv_t *kv,
                                            const lxb_char_t *data,
                                            const lxb_char_t *end)
{
    while (data < end) {
        if (lexbor_str_res_map_num[*data] == LEXBOR_STR_RES_SLIP) {
            if (kv->num_negative) {
                kv->num_exponent -= kv->num_decimals;
            }
            else {
                kv->num_exponent += kv->num_decimals;
            }

            double p10 = 10.;
            int n = kv->num_exponent;
            unit_kv_number_t *num = &kv->token->value.num;

            num->value.f = (double) kv->num_digits;

            if (n < 0) {
                n = -n;

                while (n != 0) {
                    if (n & 1) {
                        num->value.f /= p10;
                    }

                    n >>= 1;
                    p10 *= p10;
                }
            }
            else {
                while (n != 0) {
                    if (n & 1) {
                        num->value.f *= p10;
                    }

                    n >>= 1;
                    p10 *= p10;
                }
            }

            kv->token->type = UNIT_KV_TOKEN_TYPE_NUMBER;
            unit_kv_token_done_m(kv, data, end);

            /* and EOF */
            if (kv->is_eof && *data == 0x00) {
                return end;
            }

            kv->state = unit_kv_state_begin;
            return data;
        }

        kv->num_decimals = (*data - 0x30) + kv->num_decimals * 10;

        data++;
    }

    return data;
}

static const lxb_char_t *
unit_kv_state_number_digit(unit_kv_t *kv,
                           const lxb_char_t *data, const lxb_char_t *end)
{
    while (data < end) {
        /* U+002E FULL STOP (.) */
        if (*data == 0x2E) {
            kv->token->value.num.is_float = true;
            kv->state = unit_kv_state_number_null_dot_digit;

            return (data + 1);
        }
        /*
         * U+0045 LATIN SMALL LETTER E (E)
         * U+0065 LATIN SMALL LETTER E (e)
         */
        else if (*data == 0x45 || *data == 0x65) {
            kv->token->value.num.is_float = false;

            if (kv->num_negative) {
                kv->token->value.num.value.l = -kv->num_digits;
            }
            else {
                kv->token->value.num.value.l = kv->num_digits;
            }

            kv->state = unit_kv_state_number_null_dot_digit_e;
            return (data + 1);
        }
        else if (lexbor_str_res_map_num[*data] == LEXBOR_STR_RES_SLIP) {
            kv->token->value.num.is_float = false;

            if (kv->num_negative) {
                kv->token->value.num.value.l = -kv->num_digits;
            }
            else {
                kv->token->value.num.value.l = kv->num_digits;
            }

            kv->token->type = UNIT_KV_TOKEN_TYPE_NUMBER;
            unit_kv_token_done_m(kv, data, end);

            /* and EOF */
            if (kv->is_eof && *data == 0x00) {
                return end;
            }

            kv->state = unit_kv_state_begin;
            return data;
        }

        kv->num_digits = (*data - 0x30) + kv->num_digits * 10;

        data++;
    }

    return data;
}

static const lxb_char_t *
unit_kv_state_start_comment(unit_kv_t *kv,
                            const lxb_char_t *data, const lxb_char_t *end)
{
    /* U+002A ASTERISK (*) */
    if (*data == 0x2A) {
        kv->state = unit_kv_state_comment;

        return (data + 1);
    }

    /* and EOF */
    kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
    kv->error_pos = data;

    return end;
}

static const lxb_char_t *
unit_kv_state_comment(unit_kv_t *kv,
                      const lxb_char_t *data, const lxb_char_t *end)
{
    while (data < end) {
        /* U+002A ASTERISK (*) */
        if (*data == 0x2A) {
            kv->state = unit_kv_state_before_end_comment;

            return (data + 1);
        }
        /* U+000A LINE FEED (LF) */
        else if (*data == 0x0A) {
            kv->line_count++;
            kv->line_begin = data;
        }

        data++;
    }

    /* and EOF */
    kv->status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
    kv->error_pos = data;

    return end;
}

static const lxb_char_t *
unit_kv_state_before_end_comment(unit_kv_t *kv,
                                 const lxb_char_t *data, const lxb_char_t *end)
{
    /* U+002F SOLIDUS (/) */
    if (*data == 0x2F) {
        kv->state = unit_kv_state_begin;

        return (data + 1);
    }

    kv->state = unit_kv_state_comment;

    return data;
}
