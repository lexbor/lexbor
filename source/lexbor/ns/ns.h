/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_NS_H
#define LEXBOR_NS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"
#include "lexbor/ns/const.h"


typedef struct {
    const char  *name;
    const char  *name_lower;
    size_t      name_len;

    const char  *link;
    size_t      link_len;

    lxb_ns_id_t ns_id;
}
lxb_ns_data_t;


LXB_API const lxb_ns_data_t *
lxb_ns_data_by_id(lxb_ns_id_t ns_id);

LXB_API const lxb_ns_data_t *
lxb_ns_data_by_name(const lxb_char_t *name, size_t len);

LXB_API const lxb_char_t *
lxb_ns_name_by_id(lxb_ns_id_t ns_id, size_t *len);

LXB_API const lxb_char_t *
lxb_ns_lower_name_by_id(lxb_ns_id_t ns_id, size_t *len);

LXB_API const lxb_char_t *
lxb_ns_link_by_id(lxb_ns_id_t ns_id, size_t *len);

LXB_API lxb_ns_id_t
lxb_ns_id_by_name(const lxb_char_t *name, size_t len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_NS_H */
