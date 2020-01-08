/*
 * Copyright (C) 2018-2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/parser_char.h"

#define LEXBOR_STR_RES_ANSI_REPLACEMENT_CHARACTER
#include "lexbor/core/str_res.h"


const lxb_char_t *lxb_html_parser_char_eof = (const lxb_char_t *) "\x00";


static const lxb_char_t *
lxb_html_parser_char_data_check_lf(lxb_html_parser_char_t *pc,
                                   lexbor_str_t *str, const lxb_char_t *data,
                                   const lxb_char_t *end);

static const lxb_char_t *
lxb_html_parser_char_data_check_lf_lcase(lxb_html_parser_char_t *pc,
                                         lexbor_str_t *str,
                                         const lxb_char_t *data,
                                         const lxb_char_t *end);


lxb_status_t
lxb_html_parser_char_process(lxb_html_parser_char_t *pc, lexbor_str_t *str,
                             const lexbor_in_node_t *in_node,
                             const lxb_char_t *data, const lxb_char_t *end)
{
    pc->status = LXB_STATUS_OK;
    pc->is_eof = false;

    if (str->data == NULL) {
        if (lexbor_in_segment(in_node, end)) {
            lexbor_str_init(str, pc->mraw, (end - data));
        }
        else {
            lexbor_str_init(str, pc->mraw, (in_node->end - data));
        }

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    while (lexbor_in_segment(in_node, end) == false) {
        while (data < in_node->end) {
            data = pc->state(pc, str, data, in_node->end);
        }

        if (pc->status != LXB_STATUS_OK) {
            return pc->status;
        }

        if (in_node->next == NULL) {
            return LXB_STATUS_ERROR;
        }

        in_node = in_node->next;
        data = in_node->begin;
    }

    while (data < end) {
        data = pc->state(pc, str, data, end);
    }

    if (pc->status != LXB_STATUS_OK) {
        return pc->status;
    }

    data = lxb_html_parser_char_eof;
    end = lxb_html_parser_char_eof + 1UL;

    pc->is_eof = true;

    while (data < end) {
        data = pc->state(pc, str, data, end);
    }

    return pc->status;
}


lxb_status_t
lxb_html_parser_char_copy(lexbor_str_t *str, lexbor_mraw_t *mraw,
                          const lexbor_in_node_t *in_node,
                          const lxb_char_t *data, const lxb_char_t *end)
{
    lxb_status_t status;

    if (str->data == NULL) {
        if (lexbor_in_segment(in_node, end)) {
            lexbor_str_init(str, mraw, (end - data));
        }
        else {
            lexbor_str_init(str, mraw, (in_node->end - data));
        }

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    while (lexbor_in_segment(in_node, end) == false) {
        status = lxb_html_str_append(str, mraw, data, (in_node->end - data));
        if (status != LXB_STATUS_OK) {
            return status;
        }

        if (in_node->next == NULL) {
            return LXB_STATUS_ERROR;
        }

        in_node = in_node->next;
        data = in_node->begin;
    }

    if (data < end) {
        status = lxb_html_str_append(str, mraw, data, (end - data));
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

/*
 * Preprocessing (replace CRLF to LF and CR to LF)
 * and replace U+0000 NULL character to U+FFFD REPLACEMENT CHARACTER.
 */
const lxb_char_t *
lxb_html_parser_char_data(lxb_html_parser_char_t *pc, lexbor_str_t *str,
                          const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *begin = data;

    while (data != end) {
        /* U+000D CARRIAGE RETURN (CR) */
        if (*data == 0x0D) {
            data++;

            pc->status = lxb_html_str_append(str, pc->mraw, begin,
                                             (data - begin));
            if (pc->status != LXB_STATUS_OK) {
                return end;
            }

            /* Replace 0x0D to 0x0A */
            str->data[ (str->length - 1) ] = 0x0A;

            if (data >= end) {
                pc->state = lxb_html_parser_char_data_check_lf;

                return data;
            }

            /* U+000A LINE FEED (LF) */
            if (*data == 0x0A) {
                data++;
            }

            begin = data;
        }
        /* U+0000 NULL */
        else if (*data == 0x00) {
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

                continue;
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
        }
        else {
            data++;
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
lxb_html_parser_char_data_check_lf(lxb_html_parser_char_t *pc,
                                   lexbor_str_t *str, const lxb_char_t *data,
                                   const lxb_char_t *end)
{
    /* U+000A LINE FEED (LF) */
    if (*data == 0x0A) {
        data++;
    }

    pc->state = lxb_html_parser_char_data;

    return data;
}

/*
 * Preprocessing (replace CRLF to LF and CR to LF) and convert lowercase
 * and replace U+0000 NULL character to U+FFFD REPLACEMENT CHARACTER.
 */
const lxb_char_t *
lxb_html_parser_char_data_lcase(lxb_html_parser_char_t *pc, lexbor_str_t *str,
                                const lxb_char_t *data, const lxb_char_t *end)
{
    const lxb_char_t *begin = data;

    while (data != end) {
        /* U+000D CARRIAGE RETURN (CR) */
        if (*data == 0x0D) {
            data++;

            begin = lexbor_str_append_lowercase(str, pc->mraw,
                                                begin, (data - begin));
            if (begin == NULL) {
                pc->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

                return end;
            }

            /* Replace 0x0D to 0x0A */
            str->data[ (str->length - 1) ] = 0x0A;

            if (data >= end) {
                pc->state = lxb_html_parser_char_data_check_lf_lcase;

                return data;
            }

            /* U+000A LINE FEED (LF) */
            if (*data == 0x0A) {
                data++;
            }

            begin = data;
        }
        /* U+0000 NULL */
        else if (*data == 0x00) {
            if (pc->is_eof) {
                return end;
            }

            if (pc->replace_null == false) {
                if (pc->drop_null) {
                    begin = lexbor_str_append_lowercase(str, pc->mraw, begin,
                                                        (data - begin));
                    if (begin == NULL) {
                        pc->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

                        return end;
                    }

                    data++;
                    begin = data;
                }
                else {
                    data++;
                }

                continue;
            }

            if (begin != data) {
                begin = lexbor_str_append_lowercase(str, pc->mraw,
                                                    begin, (data - begin));
                if (begin == NULL) {
                    pc->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

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
        }
        else {
            data++;
        }
    }

    if (begin != data) {
        begin = lexbor_str_append_lowercase(str, pc->mraw,
                                            begin, (data - begin));
        if (begin == NULL) {
            pc->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

            return end;
        }
    }

    return data;
}

static const lxb_char_t *
lxb_html_parser_char_data_check_lf_lcase(lxb_html_parser_char_t *pc,
                                         lexbor_str_t *str,
                                         const lxb_char_t *data,
                                         const lxb_char_t *end)
{
    /* U+000A LINE FEED (LF) */
    if (*data == 0x0A) {
        data++;
    }

    pc->state = lxb_html_parser_char_data_lcase;

    return data;
}
