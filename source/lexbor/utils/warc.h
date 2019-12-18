/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LXB_UTILS_WARC_H
#define LXB_UTILS_WARC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/utils/base.h"

#include "lexbor/core/mraw.h"
#include "lexbor/core/str.h"
#include "lexbor/core/array_obj.h"


typedef struct lxb_utils_warc lxb_utils_warc_t;

typedef struct {
    lexbor_str_t type;
    double       number;
}
lxb_utils_warc_version_t;

typedef struct {
    lexbor_str_t name;
    lexbor_str_t value;
}
lxb_utils_warc_field_t;

typedef lxb_status_t
(*lxb_utils_warc_header_cb_f)(lxb_utils_warc_t *warc);

typedef lxb_status_t
(*lxb_utils_warc_content_cb_f)(lxb_utils_warc_t *warc, const lxb_char_t *data,
                               const lxb_char_t *end);
typedef lxb_status_t
(*lxb_utils_warc_content_end_cb_f)(lxb_utils_warc_t *warc);

struct lxb_utils_warc {
    lexbor_mraw_t                   *mraw;
    lexbor_array_obj_t              *fields;

    lexbor_str_t                    tmp;
    lxb_utils_warc_version_t        version;

    lxb_utils_warc_header_cb_f      header_cb;
    lxb_utils_warc_content_cb_f     content_cb;
    lxb_utils_warc_content_end_cb_f content_end_cb;
    void                            *ctx;

    size_t                          content_length;
    size_t                          content_read;
    size_t                          count;

    const char                      *error;
    unsigned                        state;
    unsigned                        ends;
    bool                            skip;
};


LXB_API lxb_utils_warc_t *
lxb_utils_warc_create(void);

LXB_API lxb_status_t
lxb_utils_warc_init(lxb_utils_warc_t *warc, lxb_utils_warc_header_cb_f h_cd,
                    lxb_utils_warc_content_cb_f c_cb,
                    lxb_utils_warc_content_end_cb_f c_end_cb, void *ctx);

LXB_API lxb_status_t
lxb_utils_warc_clear(lxb_utils_warc_t *warc);

LXB_API lxb_utils_warc_t *
lxb_utils_warc_destroy(lxb_utils_warc_t *warc, bool self_destroy);


LXB_API lxb_status_t
lxb_utils_warc_parse_file(lxb_utils_warc_t *warc, FILE *fh);

/*
 * We must call lxb_warc_parse_eof after processing.
 * Before new processing we must call lxb_warc_clear
 * if previously ends with error.
 */
LXB_API lxb_status_t
lxb_utils_warc_parse(lxb_utils_warc_t *warc,
                     const lxb_char_t **data, const lxb_char_t *end);

LXB_API lxb_status_t
lxb_utils_warc_parse_eof(lxb_utils_warc_t *warc);


LXB_API lxb_utils_warc_field_t *
lxb_utils_warc_header_field(lxb_utils_warc_t *warc, const lxb_char_t *name,
                            size_t len, size_t offset);

LXB_API lxb_status_t
lxb_utils_warc_header_serialize(lxb_utils_warc_t *warc, lexbor_str_t *str);

/*
 * Inline functions
 */
lxb_inline size_t
lxb_utils_warc_content_length(lxb_utils_warc_t *warc)
{
    return warc->content_length;
}

/*
 * No inline functions for ABI.
 */
LXB_API size_t
lxb_utils_warc_content_length_noi(lxb_utils_warc_t *warc);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LXB_UTILS_WARC_H */

