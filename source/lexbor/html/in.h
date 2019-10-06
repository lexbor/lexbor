/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_IN_H
#define LEXBOR_HTML_IN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/in.h"
#include "lexbor/core/str.h"

#include "lexbor/html/base.h"
#include "lexbor/tag/tag.h"


LXB_API lxb_status_t
lxb_html_in_make(lexbor_in_node_t *node,
                 const lxb_char_t *begin, const lxb_char_t *end,
                 lexbor_str_t *str, lexbor_mraw_t *mraw);

LXB_API lxb_tag_id_t
lxb_html_in_tag_id(lexbor_in_node_t *node, lxb_tag_heap_t *tag_heap,
                   const lxb_char_t *begin, const lxb_char_t *end,
                   lexbor_mraw_t *mraw);

LXB_API bool
lxb_html_in_ncasecmp(lexbor_in_node_t *node,
                     const lxb_char_t *begin, const lxb_char_t *end,
                     const lxb_char_t *data, size_t len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_IN_H */
