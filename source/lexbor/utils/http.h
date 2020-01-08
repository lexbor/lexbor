/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 *
 * This code only for parsing HTTP/1.x header and split with body.
 * Body parsing is not completed.
 */

#ifndef LXB_UTILS_HTTP_H
#define LXB_UTILS_HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/utils/base.h"

#include "lexbor/core/mraw.h"
#include "lexbor/core/str.h"
#include "lexbor/core/array_obj.h"


typedef struct {
    lexbor_str_t name;
    double       number;
    unsigned     status;
}
lxb_utils_http_version_t;

typedef struct {
    lexbor_str_t name;
    lexbor_str_t value;
}
lxb_utils_http_field_t;

typedef struct {
    lexbor_mraw_t            *mraw;
    lexbor_array_obj_t       *fields;

    lexbor_str_t             tmp;
    lxb_utils_http_version_t version;

    const char               *error;
    unsigned                 state;
}
lxb_utils_http_t;


LXB_API lxb_utils_http_t *
lxb_utils_http_create(void);

LXB_API lxb_status_t
lxb_utils_http_init(lxb_utils_http_t *http, lexbor_mraw_t *mraw);

LXB_API lxb_status_t
lxb_utils_http_clear(lxb_utils_http_t *http);

LXB_API lxb_utils_http_t *
lxb_utils_http_destroy(lxb_utils_http_t *http, bool self_destroy);

/*
 * Before new processing we must call lxb_utils_http_clear function.
 */
LXB_API lxb_status_t
lxb_utils_http_parse(lxb_utils_http_t *http,
                     const lxb_char_t **data, const lxb_char_t *end);

LXB_API lxb_status_t
lxb_utils_http_header_parse_eof(lxb_utils_http_t *http);


LXB_API lxb_utils_http_field_t *
lxb_utils_http_header_field(lxb_utils_http_t *http, const lxb_char_t *name,
                            size_t len, size_t offset);

LXB_API lxb_status_t
lxb_utils_http_header_serialize(lxb_utils_http_t *http, lexbor_str_t *str);

LXB_API lxb_status_t
lxb_utils_http_field_serialize(lxb_utils_http_t *http, lexbor_str_t *str,
                               const lxb_utils_http_field_t *field);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UTILS_LXB_HTTP_H */

