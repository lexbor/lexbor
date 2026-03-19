/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_COMMON_H
#define LEXBOR_HTML_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/base.h"
#include "lexbor/core/utils.h"


LXB_API lxb_status_t
lxb_html_common_parsing_integer(const lxb_char_t *data, size_t length,
                                int64_t *result);

LXB_API lxb_status_t
lxb_html_common_parsing_nonneg_integer(const lxb_char_t *data, size_t length,
                                       int64_t *result);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_COMMON_H */
