/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_STYLE_H
#define LEXBOR_HTML_STYLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/avl.h"
#include "lexbor/css/selectors/selector.h"


typedef struct {
    lexbor_avl_node_t              entry;
    lxb_css_selector_specificity_t sp;
}
lxb_html_style_node_t;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_STYLE_H */
