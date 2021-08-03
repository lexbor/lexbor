/*
 * Copyright (C) 2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LXB_CSS_STYLESHEET_H
#define LXB_CSS_STYLESHEET_H

#include "lexbor/css/node.h"
#include "lexbor/core/mraw.h"


typedef struct lxb_css_stylesheet lxb_css_stylesheet_t;

struct lxb_css_stylesheet {
    lxb_css_node_t node;

    lexbor_mraw_t  *mraw;
};


/*
 * Inline functions
 */
lxb_inline lxb_css_stylesheet_t *
lxb_css_stylesheet_create(lexbor_mraw_t *mraw)
{
//    if (mraw == NULL) {
//        mraw =
//    }

    return lexbor_mraw_calloc(mraw, sizeof(lxb_css_stylesheet_t));
}


#endif /* LXB_CSS_STYLESHEET_H */
