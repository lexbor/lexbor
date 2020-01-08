/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/token.h"
#include "lexbor/html/tokenizer.h"

#define LEXBOR_STR_RES_MAP_LOWERCASE
#define LEXBOR_STR_RES_ANSI_REPLACEMENT_CHARACTER
#define LEXBOR_STR_RES_MAP_HEX
#define LEXBOR_STR_RES_MAP_NUM
#include "lexbor/core/str_res.h"


const lxb_char_t *lxb_html_token_process_eof = (const lxb_char_t *) "\x00";


lxb_inline void
lxb_html_token_process_save_pos(lxb_html_token_process_t *process,
                                const lxb_char_t *data);

lxb_inline const lxb_char_t *
lxb_html_token_process_restore_pos(lxb_html_token_process_t *process);

static const lxb_char_t *
lxb_html_token_process_state_amp(lxb_html_token_process_t *process,
                                 const lxb_char_t *data, const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_process_state_numeric(lxb_html_token_process_t *process,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_process_state_hexademical_start(lxb_html_token_process_t *process,
                                               const lxb_char_t *data,
                                               const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_process_state_decimal_start(lxb_html_token_process_t *process,
                                           const lxb_char_t *data,
                                           const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_process_state_hexademical(lxb_html_token_process_t *process,
                                         const lxb_char_t *data,
                                         const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_process_state_decimal(lxb_html_token_process_t *process,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_skip_ws_begin_state_before(lxb_html_token_process_t *process,
                                          const lxb_char_t *data,
                                          const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_skip_ws_begin_state_end(lxb_html_token_process_t *process,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_skip_one_newline_state_before(lxb_html_token_process_t *process,
                                             const lxb_char_t *data,
                                             const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_skip_one_newline_state_cr(lxb_html_token_process_t *process,
                                         const lxb_char_t *data,
                                         const lxb_char_t *end);

static const lxb_char_t *
lxb_html_token_skip_one_newline_state_end(lxb_html_token_process_t *process,
                                          const lxb_char_t *data,
                                          const lxb_char_t *end);

const lxb_tag_data_t *
lxb_tag_append_lower(lexbor_hash_t *hash,
                     const lxb_char_t *name, size_t length);


lxb_html_token_t *
lxb_html_token_create(lexbor_dobject_t *dobj)
{
    return lexbor_dobject_calloc(dobj);
}

lxb_html_token_t *
lxb_html_token_destroy(lxb_html_token_t *token, lexbor_dobject_t *dobj)
{
    return lexbor_dobject_free(dobj, token);
}

lxb_html_token_attr_t *
lxb_html_token_attr_append(lxb_html_token_t *token, lexbor_dobject_t *dobj)
{
    lxb_html_token_attr_t *attr = lxb_html_token_attr_create(dobj);
    if (attr == NULL) {
        return NULL;
    }

    if (token->attr_last == NULL) {
        token->attr_first = attr;
        token->attr_last = attr;

        return attr;
    }

    token->attr_last->next = attr;
    attr->prev = token->attr_last;

    token->attr_last = attr;

    return attr;
}

void
lxb_html_token_attr_remove(lxb_html_token_t *token, lxb_html_token_attr_t *attr)
{
    if (token->attr_first == attr) {
        token->attr_first = attr->next;
    }

    if (token->attr_last == attr) {
        token->attr_last = attr->prev;
    }

    if (attr->next != NULL) {
        attr->next->prev = attr->prev;
    }

    if (attr->prev != NULL) {
        attr->prev->next = attr->next;
    }

    attr->next = NULL;
    attr->prev = NULL;
}

void
lxb_html_token_attr_delete(lxb_html_token_t *token,
                           lxb_html_token_attr_t *attr, lexbor_dobject_t *dobj)
{
    lxb_html_token_attr_remove(token, attr);
    lxb_html_token_attr_destroy(attr, dobj);
}

size_t
lxb_html_token_data_calc_length(lxb_html_token_t *token)
{
    size_t len = 0;
    lexbor_in_node_t *node = token->in_begin;
    const lxb_char_t *begin = token->begin;

    while (begin < token->end || node->end > token->end) {
        len += node->end - begin;

        if (node->next == NULL) {
            return 0;
        }

        node = node->next;
        begin = node->begin;
    }

    len += token->end - begin;

    return len;
}

lxb_status_t
lxb_html_token_make_data(lxb_html_token_t *token, lexbor_str_t *str,
                         lexbor_mraw_t *mraw)
{
    lexbor_in_node_t *node;
    const lxb_char_t *begin;

    if (str->data == NULL) {
        lexbor_str_init(str, mraw, 8);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    node = token->in_begin;
    begin = token->begin;

    while (lexbor_in_segment(node, token->end) == false) {
        if (lexbor_str_append(str, mraw, begin, node->end - begin) == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        if (node->next == NULL) {
            return LXB_STATUS_ERROR;
        }

        node = node->next;
        begin = node->begin;
    }

    if(lexbor_str_append(str, mraw, begin, token->end - begin) == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_token_make_data_strict(lxb_html_token_t *token, lexbor_str_t *str,
                                lexbor_mraw_t *mraw)
{
    lxb_char_t *res;
    lexbor_in_node_t *node;
    const lxb_char_t *begin, *begin_pos;

    if (str->data == NULL) {
        lexbor_str_init(str, mraw, 8);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    node = token->in_begin;
    begin = token->begin;

    while (lexbor_in_segment(node, token->end) == false) {
        begin_pos = begin;

        while (begin < node->end) {
            /* U+0000 NULL to U+FFFD REPLACEMENT CHARACTER */
            if (*begin == 0x00) {
                if (begin_pos != begin) {
                    res = lexbor_str_append_lowercase(str, mraw, begin_pos,
                                                      (begin - begin_pos));
                    if (res == NULL) {
                        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    }
                }

                res = lexbor_str_append(str, mraw,
                       lexbor_str_res_ansi_replacement_character,
                       (sizeof(lexbor_str_res_ansi_replacement_character) - 1));

                if (res == NULL) {
                    return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                }

                begin++;
                begin_pos = begin;
            }
            else {
                begin++;
            }
        }

        if (begin_pos != begin) {
            res = lexbor_str_append_lowercase(str, mraw, begin_pos,
                                              (begin - begin_pos));
            if (res == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }

        if (node->next == NULL) {
            return LXB_STATUS_ERROR;
        }

        node = node->next;
        begin = node->begin;
    }

    begin_pos = begin;

    while (begin < token->end) {
        /* U+0000 NULL to U+FFFD REPLACEMENT CHARACTER */
        if (*begin == 0x00) {
            if (begin_pos != begin) {
                res = lexbor_str_append_lowercase(str, mraw, begin_pos,
                                                  (begin - begin_pos));
                if (res == NULL) {
                    return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                }
            }

            res = lexbor_str_append(str, mraw,
                       lexbor_str_res_ansi_replacement_character,
                       (sizeof(lexbor_str_res_ansi_replacement_character) - 1));

            if (res == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            begin++;
            begin_pos = begin;
        }
        else {
            begin++;
        }
    }

    if (begin_pos != begin) {
        res = lexbor_str_append_lowercase(str, mraw, begin_pos,
                                          (begin - begin_pos));
        if (res == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_token_parse_data(lxb_html_token_t *token, lxb_html_parser_char_t *pc,
                          lexbor_str_t *str, lexbor_mraw_t *mraw)
{
    pc->mraw = mraw;

    if (token->type & LXB_HTML_TOKEN_TYPE_TEXT) {
        pc->state = lxb_html_parser_char_ref_data;
        pc->is_attribute = false;
        pc->replace_null = false;
    }
    else if (token->type & LXB_HTML_TOKEN_TYPE_DATA) {
        pc->state = lxb_html_parser_char_data;
        pc->replace_null = true;
    }
    else if (token->type & LXB_HTML_TOKEN_TYPE_RCDATA) {
        pc->state = lxb_html_parser_char_ref_data;
        pc->is_attribute = false;
        pc->replace_null = true;
    }
    else if (token->type & LXB_HTML_TOKEN_TYPE_CDATA) {
        pc->state = lxb_html_parser_char_data;
        pc->replace_null = false;
    }
    else {
        return LXB_STATUS_ERROR;
    }

    return lxb_html_parser_char_process(pc, str, token->in_begin,
                                        token->begin, token->end);
}

lxb_tag_id_t
lxb_html_token_tag_id_from_data(lexbor_hash_t *hash, lxb_html_token_t *token,
                                lexbor_mraw_t *mraw)
{
    const lxb_tag_data_t *tag_data;

    if (lexbor_in_segment(token->in_begin, token->end) == true
        && (token->type & LXB_HTML_TOKEN_TYPE_NULL) == 0)
    {
        tag_data = lxb_tag_append_lower(hash, token->begin,
                                        (token->end - token->begin));
        if (tag_data == NULL) {
            return LXB_TAG__UNDEF;
        }

        return tag_data->tag_id;
    }

    lxb_status_t status;
    lexbor_str_t str = {0};

    status = lxb_html_token_make_data_strict(token, &str, mraw);
    if (status != LXB_STATUS_OK) {
        return LXB_TAG__UNDEF;
    }

    tag_data = lxb_tag_append_lower(hash, str.data, str.length);

    lexbor_str_destroy(&str, mraw, false);

    if (tag_data == NULL) {
        return LXB_TAG__UNDEF;
    }

    return tag_data->tag_id;
}

lxb_status_t
lxb_html_token_process_data(lxb_html_token_process_t *process,
                            lxb_html_token_t *token)
{
    process->token = token;
    process->is_eof = false;

    while (lexbor_in_segment(token->in_begin, token->end) == false)
    {
        while (token->begin < token->in_begin->end) {
            token->begin = process->state(process, token->begin,
                                          token->in_begin->end);

            if (process->status == LXB_STATUS_STOPPED) {
                if (token->in_begin->next == NULL) {
                    return LXB_STATUS_ERROR;
                }

                if (token->begin == token->in_begin->end) {
                    token->in_begin = token->in_begin->next;
                    token->begin = token->in_begin->begin;
                }

                return LXB_STATUS_OK;
            }
        }

        if (process->status != LXB_STATUS_OK) {
            return process->status;
        }

        if (token->in_begin->next == NULL) {
            return LXB_STATUS_ERROR;
        }

        token->in_begin = token->in_begin->next;
        token->begin = token->in_begin->begin;
    }

    while (token->begin < token->end) {
        token->begin = process->state(process, token->begin, token->end);

        if (process->status == LXB_STATUS_STOPPED) {
            return LXB_STATUS_OK;
        }
    }

    if (process->status != LXB_STATUS_OK) {
        return process->status;
    }

    const lxb_char_t *data = lxb_html_token_process_eof;
    const lxb_char_t *end = lxb_html_token_process_eof + 1UL;

    process->is_eof = true;

    while (data < end) {
        data = process->state(process, data, end);

        if (process->status == LXB_STATUS_STOPPED) {
            return LXB_STATUS_OK;
        }
    }

    return process->status;
}

lxb_inline void
lxb_html_token_process_save_pos(lxb_html_token_process_t *process,
                                const lxb_char_t *data)
{
    process->token->begin = data;
    process->tmp_token = *process->token;
}

lxb_inline const lxb_char_t *
lxb_html_token_process_restore_pos(lxb_html_token_process_t *process)
{
    *process->token = process->tmp_token;

    return process->token->begin;
}

static const lxb_char_t *
lxb_html_token_process_state_amp(lxb_html_token_process_t *process,
                                 const lxb_char_t *data, const lxb_char_t *end)
{
    /* U+0023 NUMBER SIGN (#) */
    if (*data == 0x23) {
        process->state = lxb_html_token_process_state_numeric;

        return (data + 1);
    }
    else {
        process->status = LXB_STATUS_STOPPED;

        return lxb_html_token_process_restore_pos(process);
    }

    return data;
}

static const lxb_char_t *
lxb_html_token_process_state_numeric(lxb_html_token_process_t *process,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end)
{
    /*
     * U+0078 LATIN SMALL LETTER X
     * U+0058 LATIN CAPITAL LETTER X
     */
    if (*data == 0x78 || *data == 0x58) {
        process->state = lxb_html_token_process_state_hexademical_start;
    }
    else {
        process->state = lxb_html_token_process_state_decimal_start;

        return data;
    }

    return (data + 1);
}

static const lxb_char_t *
lxb_html_token_process_state_hexademical_start(lxb_html_token_process_t *process,
                                               const lxb_char_t *data,
                                               const lxb_char_t *end)
{
    /* ASCII hex digit */
    if (lexbor_str_res_map_hex[ *data ] != LEXBOR_STR_RES_SLIP) {
        process->state = lxb_html_token_process_state_hexademical;
    }
    else {
        process->status = LXB_STATUS_STOPPED;

        return lxb_html_token_process_restore_pos(process);
    }

    return data;
}

static const lxb_char_t *
lxb_html_token_process_state_decimal_start(lxb_html_token_process_t *process,
                                           const lxb_char_t *data,
                                           const lxb_char_t *end)
{
    /* ASCII hex digit */
    if (lexbor_str_res_map_num[ *data ] != LEXBOR_STR_RES_SLIP) {
        process->state = lxb_html_token_process_state_decimal;
    }
    else {
        process->status = LXB_STATUS_STOPPED;

        return lxb_html_token_process_restore_pos(process);
    }

    return data;
}

static const lxb_char_t *
lxb_html_token_process_state_hexademical(lxb_html_token_process_t *process,
                                         const lxb_char_t *data,
                                         const lxb_char_t *end)
{
    while (data != end) {
        if (lexbor_str_res_map_hex[ *data ] == LEXBOR_STR_RES_SLIP) {
            process->state = process->return_state;

            if(*data == ';') {
                data++;
            }

            return process->end_state(process, data, end);
        }

        if (process->num <= 0x10FFFF) {
            process->num <<= 4;
            process->num |= lexbor_str_res_map_hex[ *data ];
        }

        data++;
    }

    return data;
}

static const lxb_char_t *
lxb_html_token_process_state_decimal(lxb_html_token_process_t *process,
                                     const lxb_char_t *data,
                                     const lxb_char_t *end)
{
    while (data != end) {
        if (lexbor_str_res_map_num[ *data ] == LEXBOR_STR_RES_SLIP) {
            process->state = process->return_state;

            if (*data == ';') {
                data++;
            }

            return process->end_state(process, data, end);
        }

        if (process->num <= 0x10FFFF) {
            process->num = lexbor_str_res_map_num[ *data ] + process->num * 10;
        }

        data++;
    }

    return data;
}

lxb_status_t
lxb_html_token_data_skip_ws_begin(lxb_html_token_t *token)
{
    lxb_html_token_process_t process = {0};

    process.state = lxb_html_token_skip_ws_begin_state_before;
    process.return_state = lxb_html_token_skip_ws_begin_state_before;
    process.end_state = lxb_html_token_skip_ws_begin_state_end;

    return lxb_html_token_process_data(&process, token);
}

static const lxb_char_t *
lxb_html_token_skip_ws_begin_state_before(lxb_html_token_process_t *process,
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

            /* U+0026 AMPERSAND (&) */
            case 0x26:
                lxb_html_token_process_save_pos(process, data);

                process->state = lxb_html_token_process_state_amp;

                return (data + 1);

            /* U+0000 NULL */
            case 0x00:
                if (process->is_eof) {
                    return end;
                }
                /* fall through */

            default:
                process->status = LXB_STATUS_STOPPED;

                return data;
        }

        data++;
    }

    return data;
}

static const lxb_char_t *
lxb_html_token_skip_ws_begin_state_end(lxb_html_token_process_t *process,
                                       const lxb_char_t *data,
                                       const lxb_char_t *end)
{
    /*
     * U+0009 CHARACTER TABULATION (tab)
     * U+000A LINE FEED (LF)
     * U+000C FORM FEED (FF)
     * U+000D CARRIAGE RETURN (CR)
     * U+0020 SPACE
     */
    switch (process->num) {
        case 0x09:
        case 0x0A:
        case 0x0C:
        case 0x0D:
        case 0x20:
            return data;

        default:
            process->status = LXB_STATUS_STOPPED;

            return lxb_html_token_process_restore_pos(process);
    }

    return data;
}

lxb_status_t
lxb_html_token_data_skip_one_newline_begin(lxb_html_token_t *token)
{
    lxb_html_token_process_t process = {0};

    process.state = lxb_html_token_skip_one_newline_state_before;
    process.return_state = lxb_html_token_skip_one_newline_state_before;
    process.end_state = lxb_html_token_skip_one_newline_state_end;

    return lxb_html_token_process_data(&process, token);
}

static const lxb_char_t *
lxb_html_token_skip_one_newline_state_before(lxb_html_token_process_t *process,
                                             const lxb_char_t *data,
                                             const lxb_char_t *end)
{
    switch (*data) {
        /* U+000A LINE FEED (LF) */
        case 0x0A:
            process->status = LXB_STATUS_STOPPED;

            return (data + 1);

        /* U+000D CARRIAGE RETURN (CR) */
        case 0x0D:
            process->state = lxb_html_token_skip_one_newline_state_cr;

            return (data + 1);

        /* U+0026 AMPERSAND (&) */
        case 0x26:
            lxb_html_token_process_save_pos(process, data);

            process->state = lxb_html_token_process_state_amp;

            return (data + 1);

        /* U+0000 NULL */
        case 0x00:
            if (process->is_eof) {
                return end;
            }
            /* fall through */

        default:
            process->status = LXB_STATUS_STOPPED;

            return data;
    }

    return data;
}

static const lxb_char_t *
lxb_html_token_skip_one_newline_state_cr(lxb_html_token_process_t *process,
                                         const lxb_char_t *data,
                                         const lxb_char_t *end)
{
    process->status = LXB_STATUS_STOPPED;

    /* U+000A LINE FEED (LF) */
    if (*data == 0x0A) {
        return (data + 1);
    }

    return data;
}

static const lxb_char_t *
lxb_html_token_skip_one_newline_state_end(lxb_html_token_process_t *process,
                                          const lxb_char_t *data,
                                          const lxb_char_t *end)
{
    process->status = LXB_STATUS_STOPPED;

    /*
     * U+000A LINE FEED (LF)
     * U+000D CARRIAGE RETURN (CR)
     */
    switch (process->num) {
        case 0x0A:
        case 0x0D:
            return data;

        default:
            return lxb_html_token_process_restore_pos(process);
    }

    return data;
}

lxb_status_t
lxb_html_token_data_split_ws_begin(lxb_html_token_t *token,
                                   lxb_html_token_t *ws_token)
{
    *ws_token = *token;

    lxb_status_t status = lxb_html_token_data_skip_ws_begin(token);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (token->begin == token->end) {
        return LXB_STATUS_OK;
    }

    if (token->begin == ws_token->begin) {
        memset(ws_token, 0, sizeof(lxb_html_token_t));

        return LXB_STATUS_OK;
    }

    ws_token->end = token->begin;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_token_doctype_parse(lxb_html_token_t *token, lexbor_mraw_t *mraw,
                             lexbor_str_t *name, lexbor_str_t *public_ident,
                             lexbor_str_t *system_ident, lexbor_str_t *id_name)
{
    /* Set all to empty string if attr not exist */
    if (token->attr_first == NULL) {
        goto set_name_pub_sys_empty;
    }

    lxb_html_token_attr_t *attr;
    lxb_status_t status;
    lxb_html_parser_char_t pc = {0};

    pc.mraw = mraw;
    pc.replace_null = true;
    pc.state = lxb_html_parser_char_data_lcase;

    /* Name */
    attr = token->attr_first;

    status = lxb_html_parser_char_process(&pc, name, attr->in_name,
                                          attr->name_begin, attr->name_end);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* PUBLIC or SYSTEM */
    attr = attr->next;
    if (attr == NULL) {
        goto set_pub_sys_empty;
    }

    pc.replace_null = false;
    pc.state = lxb_html_parser_char_data_lcase;

    /* Get name: system or public */
    status = lxb_html_parser_char_process(&pc, id_name, attr->in_name,
                                          attr->name_begin, attr->name_end);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (id_name->length != 6) {
        goto set_pub_sys_empty;
    }

    pc.replace_null = true;
    pc.state = lxb_html_parser_char_data;

    if (strncmp("public",
                (const char *) id_name->data, id_name->length) == 0)
    {
        lexbor_str_init(public_ident, mraw, 0);
        if (public_ident->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        if (attr->value_begin == NULL) {
            return LXB_STATUS_OK;
        }

        status = lxb_html_parser_char_process(&pc, public_ident, attr->in_value,
                                            attr->value_begin, attr->value_end);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }
    else if (strncmp("system",
                     (const char *) id_name->data, id_name->length) == 0)
    {
        lexbor_str_init(system_ident, mraw, 0);
        if (system_ident->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        if (attr->value_begin == NULL) {
            return LXB_STATUS_OK;
        }

        status = lxb_html_parser_char_process(&pc, system_ident, attr->in_value,
                                            attr->value_begin, attr->value_end);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        return LXB_STATUS_OK;
    }
    else {
        goto set_pub_sys_empty;
    }

    /* SUSTEM */
    attr = attr->next;
    if (attr == NULL) {
        goto set_sys_empty;
    }

    pc.state = lxb_html_parser_char_data;

    status = lxb_html_parser_char_process(&pc, system_ident, attr->in_value,
                                          attr->value_begin, attr->value_end);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return LXB_STATUS_OK;

set_name_pub_sys_empty:

    lexbor_str_init(name, mraw, 0);
    if (name->data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

set_pub_sys_empty:

    lexbor_str_init(public_ident, mraw, 0);
    if (public_ident->data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

set_sys_empty:

    lexbor_str_init(system_ident, mraw, 0);
    if (system_ident->data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_HTML_STATUS_OK;
}

lxb_status_t
lxb_html_token_find_attr(lxb_html_token_t *token, lxb_html_parser_char_t *pc,
                         lexbor_mraw_t *mraw, lxb_html_token_attr_t **ret_attr,
                         const lxb_char_t *name, size_t name_len)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_token_attr_t *attr = token->attr_first;

    while (attr != NULL) {
        str.length = 0;

        status = lxb_html_token_attr_make_name(attr, &str, mraw);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        if (str.length == name_len
            && lexbor_str_data_cmp(name, str.data))
        {
            *ret_attr = attr;

            goto done;
        }

        attr = attr->next;
    }

done:

    lexbor_str_destroy(&str, mraw, false);

    return LXB_STATUS_OK;
}

/*
 * No inline functions for ABI.
 */
void
lxb_html_token_clean_noi(lxb_html_token_t *token)
{
    lxb_html_token_clean(token);
}

lxb_html_token_t *
lxb_html_token_create_eof_noi(lexbor_dobject_t *dobj)
{
    return lxb_html_token_create_eof(dobj);
}
