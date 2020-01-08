/*
 * Copyright (C) 2018-2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_PARSER_CHAR_H
#define LEXBOR_HTML_PARSER_CHAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/in.h"
#include "lexbor/core/str.h"
#include "lexbor/core/sbst.h"
#include "lexbor/core/array_obj.h"

#include "lexbor/html/base.h"


typedef struct lxb_html_parser_char lxb_html_parser_char_t;

/* State */
typedef const lxb_char_t *
(*lxb_html_parser_char_state_f)(lxb_html_parser_char_t *pc, lexbor_str_t *str,
                                const lxb_char_t *data, const lxb_char_t *end);

struct lxb_html_parser_char {
    /* It is necessary to initialize before use */
    lxb_html_parser_char_state_f     state;
    lexbor_mraw_t                    *mraw;

    bool                             replace_null;
    bool                             drop_null;
    bool                             is_attribute;

    /* Do not change out! Internal variables! */
    union {
        size_t        len;
        unsigned long num;
    }
    tmp;

    lxb_status_t                     status;
    bool                             is_eof;

    /* Parse error */
    lexbor_array_obj_t               *parse_errors;

    /* Entities */
    const lexbor_sbst_entry_static_t *entity;
    const lexbor_sbst_entry_static_t *entity_match;
    const lxb_char_t                 *entity_begin;
    size_t                           entity_str_len;
};


/*
 * Required 'pc' filled:
 * For all state handlers:
 *     .state           Parsing handle
 *     .mraw            Memory pool for string (str)
 *     .replace_null    Replace '\0' to Replacement Character '\xEF\xBF\xBD'
 *
 * For lxb_html_parser_char_ref_data:
 *     .is_attribute    Enables rules for processing entity as attribute value
 */
LXB_API lxb_status_t
lxb_html_parser_char_process(lxb_html_parser_char_t *pc, lexbor_str_t *str,
                             const lexbor_in_node_t *in_node,
                             const lxb_char_t *data, const lxb_char_t *end);

LXB_API lxb_status_t
lxb_html_parser_char_copy(lexbor_str_t *str, lexbor_mraw_t *mraw,
                          const lexbor_in_node_t *in_node,
                          const lxb_char_t *data, const lxb_char_t *end);

LXB_API const lxb_char_t *
lxb_html_parser_char_data(lxb_html_parser_char_t *cr, lexbor_str_t *str,
                          const lxb_char_t *data, const lxb_char_t *end);

LXB_API const lxb_char_t *
lxb_html_parser_char_data_lcase(lxb_html_parser_char_t *pc, lexbor_str_t *str,
                                const lxb_char_t *data, const lxb_char_t *end);

LXB_API const lxb_char_t *
lxb_html_parser_char_ref_data(lxb_html_parser_char_t *pc, lexbor_str_t *str,
                              const lxb_char_t *data, const lxb_char_t *end);


/*
 * Inline functions
 */
lxb_inline lxb_status_t
lxb_html_str_append(lexbor_str_t *str, lexbor_mraw_t *mraw,
                    const lxb_char_t *buff, size_t length)
{
    lexbor_str_check_size_arg_m(str, lexbor_str_size(str), mraw, (length + 1),
                                LXB_STATUS_ERROR_MEMORY_ALLOCATION);

    memcpy((str->data + str->length), buff, length);

    str->length += length;
    str->data[str->length] = '\0';

    return LXB_STATUS_OK;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_PARSER_CHAR_H */
