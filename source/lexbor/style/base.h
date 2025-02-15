/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_BASE_H
#define LEXBOR_STYLE_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"
#include "lexbor/core/avl.h"
#include "lexbor/html/html.h"
#include "lexbor/css/css.h"
#include "lexbor/selectors/selectors.h"


#define LXB_STYLE_VERSION_MAJOR 0
#define LXB_STYLE_VERSION_MINOR 1
#define LXB_STYLE_VERSION_PATCH 0

#define LXB_STYLE_VERSION_STRING LEXBOR_STRINGIZE(LXB_STYLE_VERSION_MAJOR) "."   \
                                 LEXBOR_STRINGIZE(LXB_STYLE_VERSION_MINOR) "."   \
                                 LEXBOR_STRINGIZE(LXB_STYLE_VERSION_PATCH)


typedef struct lxb_style_weak lxb_style_weak_t;

struct lxb_style_weak {
    void                           *value;
    lxb_css_selector_specificity_t sp;

    lxb_style_weak_t               *next;
};

typedef struct {
    lexbor_avl_node_t              entry;
    lxb_style_weak_t               *weak;

    lxb_css_selector_specificity_t sp;
}
lxb_style_node_t;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_BASE_H */
