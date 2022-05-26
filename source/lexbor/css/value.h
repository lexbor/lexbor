/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LXB_CSS_VALUE_H
#define LXB_CSS_VALUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/css/base.h"
#include "lexbor/css/value/const.h"
#include "lexbor/css/unit/const.h"


typedef struct {
    double              num;
    lxb_css_unit_t      unit;
    bool                is_float;
}
lxb_css_value_length_t;

typedef struct {
    double num;
    bool   is_float;
}
lxb_css_value_percentage_t;


LXB_API lxb_css_value_type_t
lxb_css_value_by_name(const lxb_char_t *name, size_t length);

LXB_API lxb_status_t
lxb_css_value_serialize(lxb_css_value_type_t type,
                        lexbor_serialize_cb_f cb, void *ctx);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LXB_CSS_VALUE_H */
