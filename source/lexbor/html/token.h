/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_HTML_TOKEN_H
#define LEXBOR_HTML_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/dobject.h"
#include "lexbor/core/in.h"
#include "lexbor/core/str.h"

#include "lexbor/html/base.h"
#include "lexbor/html/token_attr.h"
#include "lexbor/html/tag.h"
#include "lexbor/html/parser_char.h"


typedef struct lxb_html_token_process lxb_html_token_process_t;

typedef const lxb_char_t *
(*lxb_html_token_process_state_f)(lxb_html_token_process_t *process,
                                 const lxb_char_t *data, const lxb_char_t *end);


typedef enum {
    LXB_HTML_TOKEN_TYPE_OPEN         = 0x0000,
    LXB_HTML_TOKEN_TYPE_CLOSE        = 0x0001,
    LXB_HTML_TOKEN_TYPE_CLOSE_SELF   = 0x0002,
    LXB_HTML_TOKEN_TYPE_TEXT         = 0x0004,
    LXB_HTML_TOKEN_TYPE_DATA         = 0x0008,
    LXB_HTML_TOKEN_TYPE_RCDATA       = 0x0010,
    LXB_HTML_TOKEN_TYPE_CDATA        = 0x0020,
    LXB_HTML_TOKEN_TYPE_NULL         = 0x0040,
    LXB_HTML_TOKEN_TYPE_FORCE_QUIRKS = 0x0080,
    LXB_HTML_TOKEN_TYPE_DONE         = 0x0100
}
lxb_html_token_type_t;

typedef struct {
    const lxb_char_t      *begin;
    const lxb_char_t      *end;

    lexbor_in_node_t      *in_begin;

    lxb_html_token_attr_t *attr_first;
    lxb_html_token_attr_t *attr_last;

    void                  *base_element;

    lxb_html_tag_id_t     tag_id;
    lxb_html_token_type_t type;
}
lxb_html_token_t;

struct lxb_html_token_process {
    lxb_html_token_process_state_f state;
    lxb_html_token_process_state_f amp_return_state;
    lxb_html_token_process_state_f amp_end_state;

    lxb_html_token_t               *token;
    lxb_html_token_t               tmp_token;

    unsigned long                  num;

    lxb_status_t                   status;
    bool                           is_eof;
};


lxb_html_token_t *
lxb_html_token_create(lexbor_dobject_t *dobj);

lxb_html_token_t *
lxb_html_token_destroy(lxb_html_token_t *token, lexbor_dobject_t *dobj);

lxb_html_token_attr_t *
lxb_html_token_attr_append(lxb_html_token_t *token, lexbor_dobject_t *dobj);

void
lxb_html_token_attr_remove(lxb_html_token_t *token,
                           lxb_html_token_attr_t *attr);

void
lxb_html_token_attr_delete(lxb_html_token_t *token,
                           lxb_html_token_attr_t *attr, lexbor_dobject_t *dobj);

size_t
lxb_html_token_data_calc_length(lxb_html_token_t *token);

lxb_status_t
lxb_html_token_make_data(lxb_html_token_t *token, lexbor_str_t *str,
                         lexbor_mraw_t *mraw);

lxb_status_t
lxb_html_token_make_data_strict(lxb_html_token_t *token, lexbor_str_t *str,
                                lexbor_mraw_t *mraw);

lxb_status_t
lxb_html_token_parse_data(lxb_html_token_t *token, lxb_html_parser_char_t *pc,
                          lexbor_str_t *str, lexbor_mraw_t *mraw);

lxb_html_tag_id_t
lxb_html_token_tag_id_from_data(lxb_html_tag_heap_t *tag_heap,
                                lxb_html_token_t *token);

lxb_status_t
lxb_html_token_process_data(lxb_html_token_process_t *process,
                            lxb_html_token_t *token);

lxb_status_t
lxb_html_token_data_skip_ws_begin(lxb_html_token_t *token);

lxb_status_t
lxb_html_token_data_skip_one_newline_begin(lxb_html_token_t *token);

lxb_status_t
lxb_html_token_data_split_ws_begin(lxb_html_token_t *token,
                                   lxb_html_token_t *ws_token);

lxb_status_t
lxb_html_token_doctype_parse(lxb_html_token_t *token, lexbor_mraw_t *mraw,
                             lexbor_str_t *name, lexbor_str_t *public,
                             lexbor_str_t *system, lexbor_str_t *id_name);

lxb_status_t
lxb_html_token_find_attr(lxb_html_token_t *token, lxb_html_parser_char_t *pc,
                         lexbor_mraw_t *mraw, lxb_html_token_attr_t **ret_attr,
                         const lxb_char_t *name, size_t name_len);


/*
 * Inline functions
 */
lxb_inline void
lxb_html_token_clean(lxb_html_token_t *token)
{
    memset(token, 0, sizeof(lxb_html_token_t));
}

lxb_inline lxb_html_token_t *
lxb_html_token_create_eof(lexbor_dobject_t *dobj)
{
    return lexbor_dobject_calloc(dobj);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_TOKEN_H */

