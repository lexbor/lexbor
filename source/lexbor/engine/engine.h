/*
 * Copyright (C) 2024-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 *
 * This module uses/will use all other modules.  All high-level API is/will be
 * implemented in this module.
 *
 * You don't need to use this module's API if you just need to parse HTML, CSS
 * or... use a specific module (lexbor/html, lexbor/css, ...) for that.
 */

#ifndef LXB_ENGINE_H
#define LXB_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/engine/base.h"
#include "lexbor/html/html.h"
#include "lexbor/encoding/encoding.h"
#include "lexbor/style/style.h"


typedef struct {
    lxb_html_document_t *document;
    lxb_html_encoding_t *html_encoding;
}
lxb_engine_t;


LXB_API lxb_engine_t *
lxb_engine_create(void);

LXB_API lxb_status_t
lxb_engine_init(lxb_engine_t *engine);

LXB_API lxb_engine_t *
lxb_engine_destroy(lxb_engine_t *engine);

LXB_API lxb_status_t
lxb_engine_parse(lxb_engine_t *engine, const lxb_char_t *html, size_t length,
                 lxb_encoding_t encoding);

LXB_API lxb_status_t
lxb_engine_encoding_from_to(const lxb_char_t *data, size_t length,
                            lxb_encoding_t from, lxb_encoding_t to,
                            lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_encoding_t
lxb_engine_encoding_from_meta(lxb_engine_t *engine, const lxb_char_t *html,
                              size_t length);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LXB_ENGINE_H */
