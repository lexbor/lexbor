/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/in.h"


lxb_status_t
lxb_html_in_make(lexbor_in_node_t *node,
                 const lxb_char_t *begin, const lxb_char_t *end,
                 lexbor_str_t *str, lexbor_mraw_t *mraw)
{
    if (str->data == NULL) {
        lexbor_str_init(str, mraw, 8);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    if (begin == NULL) {
        return LXB_STATUS_OK;
    }

    while (lexbor_in_segment(node, end) == false) {
        if (lexbor_str_append(str, mraw, begin, node->end - begin) == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        if (node->next == NULL) {
            return LXB_STATUS_ERROR;
        }

        node = node->next;
        begin = node->begin;
    }

    if(lexbor_str_append(str, mraw, begin, end - begin) == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

lxb_tag_id_t
lxb_html_in_tag_id(lexbor_in_node_t *node, lexbor_hash_t *hash,
                   const lxb_char_t *begin, const lxb_char_t *end,
                   lexbor_mraw_t *mraw)
{
    if (lexbor_in_segment(node, end)) {
        return lxb_tag_id_by_name(hash, begin, (end - begin));
    }

    lexbor_str_t str = {0};
    lxb_tag_id_t tag_id;

    lxb_status_t status = lxb_html_in_make(node, begin, end, &str, mraw);
    if (status != LXB_STATUS_OK) {
        return LXB_TAG__UNDEF;
    }

    tag_id = lxb_tag_id_by_name(hash, str.data, str.length);

    lexbor_str_destroy(&str, mraw, false);

    return tag_id;
}

bool
lxb_html_in_ncasecmp(lexbor_in_node_t *node,
                     const lxb_char_t *begin, const lxb_char_t *end,
                     const lxb_char_t *data, size_t len)
{
    if (lexbor_in_segment(node, end)) {
        if ((end - begin) != len) {
            return false;
        }

        return lexbor_str_data_ncasecmp(begin, data, len);
    }

    size_t n;

    for (;;) {
        n = node->end - begin;

        if (n > len) {
            n = len;
        }

        if (lexbor_str_data_ncasecmp(begin, data, n) == false) {
            return false;
        }

        len -= n;

        if (len == 0) {
            return true;
        }

        if (node->next == NULL) {
            return false;
        }

        data = &data[n];
        node = node->next;
        begin = node->begin;
    }

    return true;
}
