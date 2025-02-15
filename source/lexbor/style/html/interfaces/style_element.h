/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_HTML_STYLE_ELEMENT_H
#define LEXBOR_STYLE_HTML_STYLE_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/style/html/interfaces/document.h"


LXB_API lxb_status_t
lxb_html_style_element_parse(lxb_html_style_element_t *element);

LXB_API lxb_status_t
lxb_html_style_element_remove(lxb_html_style_element_t *element);

LXB_API lxb_status_t
lxb_html_style_element_cb(lxb_html_style_element_t *style);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_HTML_STYLE_ELEMENT_H */
