/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/token_attr.h"
#include "lexbor/html/in.h"


lxb_html_token_attr_t *
lxb_html_token_attr_create(lexbor_dobject_t *dobj)
{
    return lexbor_dobject_calloc(dobj);
}

void
lxb_html_token_attr_clean(lxb_html_token_attr_t *attr)
{
    memset(attr, 0, sizeof(lxb_html_token_attr_t));
}

lxb_html_token_attr_t *
lxb_html_token_attr_destroy(lxb_html_token_attr_t *attr, lexbor_dobject_t *dobj)
{
    return lexbor_dobject_free(dobj, attr);
}

lxb_status_t
lxb_html_token_attr_make_name(lxb_html_token_attr_t *attr, lexbor_str_t *str,
                              lexbor_mraw_t *mraw)
{
    return lxb_html_in_make(attr->in_name, attr->name_begin, attr->name_end,
                            str, mraw);
}

lxb_status_t
lxb_html_token_attr_make_value(lxb_html_token_attr_t *attr, lexbor_str_t *str,
                               lexbor_mraw_t *mraw)
{
    return lxb_html_in_make(attr->in_value, attr->value_begin, attr->value_end,
                            str, mraw);
}

lxb_status_t
lxb_html_token_attr_parse(lxb_html_token_attr_t *attr,
                          lxb_html_parser_char_t *pc, lexbor_str_t *name,
                          lexbor_str_t *value, lexbor_mraw_t *mraw)
{
    lxb_status_t status = LXB_STATUS_ERROR;

    pc->mraw = mraw;
    pc->replace_null = true;
    pc->is_attribute = true;
    pc->state = lxb_html_parser_char_data_lcase;

    if (attr->name_begin != NULL) {
        status = lxb_html_parser_char_process(pc, name, attr->in_name,
                                              attr->name_begin, attr->name_end);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }
    else {
        lexbor_str_init(name, mraw, 0);
        if (name->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    if (attr->value_begin != NULL) {
        pc->state = lxb_html_parser_char_ref_data;

        status = lxb_html_parser_char_process(pc, value, attr->in_value,
                                            attr->value_begin, attr->value_end);
    }

    return status;
}
