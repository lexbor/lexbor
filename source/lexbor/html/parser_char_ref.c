/*
 * Copyright (C) 2018-2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/parser_char.h"
#include "lexbor/html/tokenizer/error.h"

#define LXB_HTML_TOKENIZER_RES_ENTITIES_SBST
#include "lexbor/html/tokenizer/res.h"

#define LEXBOR_STR_RES_ANSI_REPLACEMENT_CHARACTER
#define LEXBOR_STR_RES_ALPHANUMERIC_CHARACTER
#define LEXBOR_STR_RES_ALPHA_CHARACTER
#define LEXBOR_STR_RES_MAP_HEX
#define LEXBOR_STR_RES_MAP_NUM
#define LEXBOR_STR_RES_REPLACEMENT_CHARACTER
#include "lexbor/core/str_res.h"


static const lxb_char_t *
lxb_html_parser_char_ref_check_lf(lxb_html_parser_char_t *pc,
                                  lexbor_str_t *str, const lxb_char_t *data,
                                  const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_ref_state(lxb_html_parser_char_t *pc,
                               lexbor_str_t *str, const lxb_char_t *data,
                               const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_ref_state_named(lxb_html_parser_char_t *pc,
                                     lexbor_str_t *str, const lxb_char_t *data,
                                     const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_ref_state_ambiguous_ampersand(lxb_html_parser_char_t *pc,
                                                   lexbor_str_t *str,
                                                   const lxb_char_t *data,
                                                   const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_ref_state_numeric(lxb_html_parser_char_t *pc,
                                       lexbor_str_t *str,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_ref_state_hexademical_start(lxb_html_parser_char_t *pc,
                                                 lexbor_str_t *str,
                                                 const lxb_char_t *data,
                                                 const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_ref_state_decimal_start(lxb_html_parser_char_t *pc,
                                             lexbor_str_t *str,
                                             const lxb_char_t *data,
                                             const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_ref_state_hexademical(lxb_html_parser_char_t *pc,
                                           lexbor_str_t *str,
                                           const lxb_char_t *data,
                                           const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_ref_state_decimal(lxb_html_parser_char_t *pc,
                                       lexbor_str_t *str,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_ref_numeric_end(lxb_html_parser_char_t *pc,
                                     lexbor_str_t *str, const lxb_char_t *data,
                                     const lxb_char_t *end);

size_t
lxb_html_parser_char_ref_numeric_to_ascii_utf_8(size_t codepoint,
                                                lxb_char_t *data);


/*
 * 12.2.5.72 Character reference state
 * with preprocessing (replace CRLF to LF and CR to LF)
 * and with replace U+0000 NULL character to U+FFFD REPLACEMENT CHARACTER.
 */
const lxb_char_t *
lxb_html_parser_char_ref_data(lxb_html_parser_char_t *pc, lexbor_str_t *str,
                              const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *begin = data;

    while (data != end) {
        switch (*data) {
            /* U+0026 AMPERSAND (&) */
            case 0x26:
                /*
                 * Append AMPERSAND (&) to str.
                 * It will be replaced if entity is correct.
                 */
                data++;

                pc->status = lxb_html_str_append(str, pc->mraw, begin,
                                                 (data - begin));
                if (pc->status != LXB_STATUS_OK) {
                    return end;
                }

                pc->state = lxb_html_parser_char_ref_state;

                return data;

            /* U+000D CARRIAGE RETURN (CR) */
            case 0x0D:
                data++;

                pc->status = lxb_html_str_append(str, pc->mraw, begin,
                                                 (data - begin));
                if (pc->status != LXB_STATUS_OK) {
                    return end;
                }

                /* Replace 0x0D to 0x0A */
                str->data[ (str->length - 1) ] = 0x0A;

                if (data >= end) {
                    pc->state = lxb_html_parser_char_ref_check_lf;

                    return data;
                }

                /* U+000A LINE FEED (LF) */
                if (*data == 0x0A) {
                    data++;
                }

                begin = data;

                break;

            /* U+0000 NULL */
            case 0x00:
                if (pc->is_eof) {
                    return end;
                }

                if (pc->replace_null == false) {
                    if (pc->drop_null) {
                        pc->status = lxb_html_str_append(str, pc->mraw, begin,
                                                         (data - begin));
                        if (pc->status != LXB_STATUS_OK) {
                            return end;
                        }

                        data++;
                        begin = data;
                    }
                    else {
                        data++;
                    }

                    break;
                }

                if (begin != data) {
                    pc->status = lxb_html_str_append(str, pc->mraw, begin,
                                                     (data - begin));
                    if (pc->status != LXB_STATUS_OK) {
                        return end;
                    }
                }

                pc->status = lxb_html_str_append(str, pc->mraw,
                         lexbor_str_res_ansi_replacement_character,
                         sizeof(lexbor_str_res_ansi_replacement_character) - 1);
                if (pc->status != LXB_STATUS_OK) {
                    return end;
                }

                data++;
                begin = data;

                break;

            default:
                data++;

                break;
        }
    }

    if (begin != data) {
        pc->status = lxb_html_str_append(str, pc->mraw, begin, (data - begin));
        if (pc->status != LXB_STATUS_OK) {
            return end;
        }
    }

    return data;
}

static const lxb_char_t *
lxb_html_parser_char_ref_check_lf(lxb_html_parser_char_t *pc, lexbor_str_t *str,
                                  const lxb_char_t *data, const lxb_char_t *end)
{
    /* U+000A LINE FEED (LF) */
    if (*data == 0x0A) {
        data++;
    }

    pc->state = lxb_html_parser_char_ref_data;

    return data;
}

/*
 * 12.2.5.72 Character reference state
 */
static const lxb_char_t *
lxb_html_parser_char_ref_state(lxb_html_parser_char_t *pc,
                               lexbor_str_t *str, const lxb_char_t *data,
                               const lxb_char_t *end)
{
    /* ASCII alphanumeric */
    if (lexbor_str_res_alphanumeric_character[ *data ] != LEXBOR_STR_RES_SLIP) {
        pc->tmp.len = str->length - 1;

        pc->entity = &lxb_html_tokenizer_res_entities_sbst[1];
        pc->entity_match = NULL;

        pc->state = lxb_html_parser_char_ref_state_named;

        return data;
    }
    /* U+0023 NUMBER SIGN (#) */
    else if (*data == 0x23) {
        pc->status = lxb_html_str_append(str, pc->mraw, data, 1);
        if (pc->status != LXB_STATUS_OK) {
            return end;
        }

        pc->entity_begin = data;
        pc->entity_str_len = str->length - 2;

        pc->state = lxb_html_parser_char_ref_state_numeric;

        return (data + 1);
    }
    else {
        pc->state = lxb_html_parser_char_ref_data;
    }

    return data;
}

/*
 * 12.2.5.73 Named character reference state
 *
 * The slowest part in HTML parsing!!!
 *
 * This option works correctly and passes all tests (stream parsing too).
 * We must seriously think about how to accelerate this part.
 */
static const lxb_char_t *
lxb_html_parser_char_ref_state_named(lxb_html_parser_char_t *pc,
                                     lexbor_str_t *str, const lxb_char_t *data,
                                     const lxb_char_t *end)
{
    const lexbor_sbst_entry_static_t *entry = pc->entity;

    pc->entity_begin = data;

    while (data != end) {
        entry = lexbor_sbst_entry_static_find(
                                           lxb_html_tokenizer_res_entities_sbst,
                                           entry, *data);
        if (entry == NULL) {
            pc->status = lxb_html_str_append(str, pc->mraw, pc->entity_begin,
                                             ((data - pc->entity_begin) + 1));
            if (pc->status != LXB_STATUS_OK) {
                return end;
            }

            goto done;
        }

        if (entry->value != NULL) {
            if (data != pc->entity_begin)
            {
                pc->status = lxb_html_str_append(str, pc->mraw, pc->entity_begin,
                                                 (data - pc->entity_begin));
                if (pc->status != LXB_STATUS_OK) {
                    return end;
                }
            }

            pc->entity_begin = data;
            pc->entity_match = entry;
            pc->entity_str_len = str->length + 1;
        }

        entry = &lxb_html_tokenizer_res_entities_sbst[ entry->next ];

        data++;
    }

    /* If entry not NULL and buffer empty, then wait next buffer. */
    pc->entity = entry;

    pc->status = lxb_html_str_append(str, pc->mraw, pc->entity_begin,
                                     (end - pc->entity_begin));
    if (pc->status != LXB_STATUS_OK) {
        return end;
    }

    return data;

done:

    /* If we have bad entity */
    if (pc->entity_match == NULL) {
        pc->state = lxb_html_parser_char_ref_state_ambiguous_ampersand;

        /*
         * We wrote down the last character for checking the attributes,
         * now we need to delete it and reconsume.
         */
        str->length--;
        str->data[str->length] = 0x00;

        return data;
    }

    pc->state = lxb_html_parser_char_ref_data;

    /*
     * If the character reference was consumed as part of an attribute,
     * and the last character matched is not a U+003B SEMICOLON character (;),
     * and the next input character is either a U+003D EQUALS SIGN character (=)
     * or an ASCII alphanumeric, then, for historical reasons,
     * flush code points consumed as a character reference
     * and switch to the return state.
     */
    /* U+003B SEMICOLON character (;) */
    if (pc->is_attribute && pc->entity_match->key != 0x3B) {
        lxb_char_t ch = str->data[ pc->entity_str_len ];

        /* U+003D EQUALS SIGN character (=) or  ASCII alphanumeric */
        if (ch == 0x3D
            || lexbor_str_res_alphanumeric_character[ch] != LEXBOR_STR_RES_SLIP)
        {
            return (data + 1);
        }
    }

    if (pc->entity_match->key == 0x3B) {
        lxb_html_tokenizer_error_add(pc->parse_errors, pc->entity_begin,
                                     LXB_HTML_TOKENIZER_ERROR_MISEAFCHRE);
    }

    /* Append entity to str buffer and reconsume data */
    if ((str->length - pc->tmp.len) > pc->entity_match->value_len) {
        size_t dest_len = (pc->tmp.len + pc->entity_match->value_len);

        memcpy(&str->data[pc->tmp.len], pc->entity_match->value,
               pc->entity_match->value_len);

        memmove(&str->data[dest_len], &str->data[pc->entity_str_len],
                (str->length - pc->entity_str_len));

        str->length -= pc->entity_str_len - dest_len;

        /*
         * ...need to delete it and reconsume.
         */
        str->length--;
        str->data[str->length] = 0x00;
    }
    else {
        str->length = pc->tmp.len;

        pc->status = lxb_html_str_append(str, pc->mraw, pc->entity_match->value,
                                         pc->entity_match->value_len);
        if (pc->status != LXB_STATUS_OK) {
            return end;
        }
    }

    return data;
}

/*
 * 12.2.5.74 Ambiguous ampersand state
 */
static const lxb_char_t *
lxb_html_parser_char_ref_state_ambiguous_ampersand(lxb_html_parser_char_t *pc,
                                                   lexbor_str_t *str,
                                                   const lxb_char_t *data,
                                                   const lxb_char_t *end)
{
    /* ASCII alphanumeric */
    /* Skipped, not need */

    /* U+003B SEMICOLON (;) */
    if (*data == 0x3B) {
        lxb_html_tokenizer_error_add(pc->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_UNNACHRE);
    }

    pc->state = lxb_html_parser_char_ref_data;

    return data;
}

/*
 * 12.2.5.75 Numeric character reference state
 */
static const lxb_char_t *
lxb_html_parser_char_ref_state_numeric(lxb_html_parser_char_t *pc,
                                       lexbor_str_t *str,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end)
{
    pc->tmp.num = 0;

    /*
     * U+0078 LATIN SMALL LETTER X
     * U+0058 LATIN CAPITAL LETTER X
     */
    if (*data == 0x78 || *data == 0x58) {
        pc->status = lxb_html_str_append(str, pc->mraw, data, 1);
        if (pc->status != LXB_STATUS_OK) {
            return end;
        }

        pc->state = lxb_html_parser_char_ref_state_hexademical_start;
    }
    else {
        pc->state = lxb_html_parser_char_ref_state_decimal_start;

        return data;
    }

    return (data + 1);
}

/*
 * 12.2.5.76 Hexademical character reference start state
 */
static const lxb_char_t *
lxb_html_parser_char_ref_state_hexademical_start(lxb_html_parser_char_t *pc,
                                                 lexbor_str_t *str,
                                                 const lxb_char_t *data,
                                                 const lxb_char_t *end)
{
    /* ASCII hex digit */
    if (lexbor_str_res_map_hex[ *data ] != LEXBOR_STR_RES_SLIP) {
        pc->state = lxb_html_parser_char_ref_state_hexademical;
    }
    else {
        lxb_html_tokenizer_error_add(pc->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_ABOFDIINNUCHRE);

        pc->state = lxb_html_parser_char_ref_data;
    }

    return data;
}

/*
 * 12.2.5.77 Decimal character reference start state
 */
static const lxb_char_t *
lxb_html_parser_char_ref_state_decimal_start(lxb_html_parser_char_t *pc,
                                             lexbor_str_t *str,
                                             const lxb_char_t *data,
                                             const lxb_char_t *end)
{
    /* ASCII digit */
    if (lexbor_str_res_map_num[ *data ] != LEXBOR_STR_RES_SLIP) {
        pc->state = lxb_html_parser_char_ref_state_decimal;
    }
    else {
        lxb_html_tokenizer_error_add(pc->parse_errors, data,
                                     LXB_HTML_TOKENIZER_ERROR_ABOFDIINNUCHRE);

        pc->state = lxb_html_parser_char_ref_data;
    }

    return data;
}

/*
 * 12.2.5.78 Hexademical character reference state
 */
static const lxb_char_t *
lxb_html_parser_char_ref_state_hexademical(lxb_html_parser_char_t *pc,
                                           lexbor_str_t *str,
                                           const lxb_char_t *data,
                                           const lxb_char_t *end)
{
    const lxb_char_t *begin = data;

    while (data != end) {
        if (lexbor_str_res_map_hex[ *data ] == LEXBOR_STR_RES_SLIP) {
            pc->state = lxb_html_parser_char_ref_data;

            if (*data == ';') {
                data++;
            }

            return lxb_html_parser_char_ref_numeric_end(pc, str, data, end);
        }

        if (pc->tmp.num <= 0x10FFFF) {
            pc->tmp.num <<= 4;
            pc->tmp.num |= lexbor_str_res_map_hex[ *data ];
        }

        data++;
    }

    pc->status = lxb_html_str_append(str, pc->mraw, begin, (data - begin));
    if (pc->status != LXB_STATUS_OK) {
        return end;
    }

    return data;
}

/*
 * 12.2.5.79 Decimal character reference state
 */
static const lxb_char_t *
lxb_html_parser_char_ref_state_decimal(lxb_html_parser_char_t *pc,
                                       lexbor_str_t *str,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end)
{
    const lxb_char_t *begin = data;

    while (data != end) {
        if (lexbor_str_res_map_num[ *data ] == LEXBOR_STR_RES_SLIP) {
            pc->state = lxb_html_parser_char_ref_data;

            if (*data == ';') {
                data++;
            }

            return lxb_html_parser_char_ref_numeric_end(pc, str, data, end);
        }

        if (pc->tmp.num <= 0x10FFFF) {
            pc->tmp.num = lexbor_str_res_map_num[ *data ] + pc->tmp.num * 10;
        }

        data++;
    }

    pc->status = lxb_html_str_append(str, pc->mraw, begin, (data - begin));
    if (pc->status != LXB_STATUS_OK) {
        return end;
    }

    return data;
}

/*
 * 12.2.5.80 Numeric character reference end state
 */
static const lxb_char_t *
lxb_html_parser_char_ref_numeric_end(lxb_html_parser_char_t *pc,
                                     lexbor_str_t *str, const lxb_char_t *data,
                                     const lxb_char_t *end)
{
    const lxb_char_t *res;

    /* Rollback string position */
    str->length = pc->entity_str_len;

    if (pc->tmp.num == 0x00) {
        lxb_html_tokenizer_error_add(pc->parse_errors, pc->entity_begin,
                                     LXB_HTML_TOKENIZER_ERROR_NUCHRE);

        goto xFFFD;
    }
    else if (pc->tmp.num > 0x10FFFF) {
        lxb_html_tokenizer_error_add(pc->parse_errors, pc->entity_begin,
                                     LXB_HTML_TOKENIZER_ERROR_CHREOUUNRA);

        goto xFFFD;
    }
    else if (pc->tmp.num >= 0xD800 && pc->tmp.num <= 0xDFFF) {
        lxb_html_tokenizer_error_add(pc->parse_errors, pc->entity_begin,
                                     LXB_HTML_TOKENIZER_ERROR_SUCHRE);

        goto xFFFD;
    }
    else if (pc->tmp.num >= 0xFDD0 && pc->tmp.num <= 0xFDEF) {
        lxb_html_tokenizer_error_add(pc->parse_errors, pc->entity_begin,
                                     LXB_HTML_TOKENIZER_ERROR_NOCHRE);
    }

    switch (pc->tmp.num) {
        case 0xFFFE:  case 0xFFFF:  case 0x1FFFE: case 0x1FFFF: case 0x2FFFE:
        case 0x2FFFF: case 0x3FFFE: case 0x3FFFF: case 0x4FFFE: case 0x4FFFF:
        case 0x5FFFE: case 0x5FFFF: case 0x6FFFE: case 0x6FFFF: case 0x7FFFE:
        case 0x7FFFF: case 0x8FFFE: case 0x8FFFF: case 0x9FFFE: case 0x9FFFF:
        case 0xAFFFE: case 0xAFFFF: case 0xBFFFE: case 0xBFFFF: case 0xCFFFE:
        case 0xCFFFF: case 0xDFFFE: case 0xDFFFF: case 0xEFFFE: case 0xEFFFF:
        case 0xFFFFE: case 0xFFFFF:
        case 0x10FFFE:
        case 0x10FFFF:
            lxb_html_tokenizer_error_add(pc->parse_errors, pc->entity_begin,
                                         LXB_HTML_TOKENIZER_ERROR_NOCHRE);

            break;

        default:
            break;
    }

    if (pc->tmp.num <= 0x1F || (pc->tmp.num >= 0x7F && pc->tmp.num <= 0x9F)) {
        lxb_html_tokenizer_error_add(pc->parse_errors, pc->entity_begin,
                                     LXB_HTML_TOKENIZER_ERROR_COCHRE);
    }

    if (pc->tmp.num <= 0x9F) {
        pc->tmp.num = lexbor_str_res_replacement_character[pc->tmp.num];
    }

    res = lexbor_str_check_size(str, pc->mraw, 5);
    if (res == NULL) {
        pc->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

        return end;
    }

    str->length += lxb_html_parser_char_ref_numeric_to_ascii_utf_8(pc->tmp.num,
                                                     &str->data[ str->length ]);

    str->data[ str->length ] = 0x00;

    return data;

xFFFD:

    pc->status = lxb_html_str_append(str, pc->mraw,
                         lexbor_str_res_ansi_replacement_character,
                         sizeof(lexbor_str_res_ansi_replacement_character) - 1);
    if (pc->status != LXB_STATUS_OK) {
        return end;
    }

    return data;
}

size_t
lxb_html_parser_char_ref_numeric_to_ascii_utf_8(size_t codepoint,
                                                lxb_char_t *data)
{
    /* 0x80 -- 10xxxxxx */
    /* 0xC0 -- 110xxxxx */
    /* 0xE0 -- 1110xxxx */
    /* 0xF0 -- 11110xxx */

    if (codepoint <= 0x0000007F) {
        /* 0xxxxxxx */
        data[0] = (char)codepoint;

        return 1;
    }
    else if (codepoint <= 0x000007FF) {
        /* 110xxxxx 10xxxxxx */
        data[0] = (char)(0xC0 | (codepoint >> 6  ));
        data[1] = (char)(0x80 | (codepoint & 0x3F));

        return 2;
    }
    else if (codepoint <= 0x0000FFFF) {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        data[0] = (char)(0xE0 | ((codepoint >> 12)));
        data[1] = (char)(0x80 | ((codepoint >> 6 ) & 0x3F));
        data[2] = (char)(0x80 | ( codepoint & 0x3F));

        return 3;
    }
    else if (codepoint <= 0x001FFFFF) {
        /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        data[0] = (char)(0xF0 | ( codepoint >> 18));
        data[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        data[2] = (char)(0x80 | ((codepoint >> 6 ) & 0x3F));
        data[3] = (char)(0x80 | ( codepoint & 0x3F));

        return 4;
    }

    return 0;
}
