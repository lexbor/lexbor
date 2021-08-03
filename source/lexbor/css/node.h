/*
 * Copyright (C) 2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LXB_CSS_NODE_H
#define LXB_CSS_NODE_H

#include "lexbor/css/base.h"


typedef enum {
    LXB_CSS_TYPE_UNDEF = 0,
    LXB_CSS_TYPE_STYLESHEET,
    LXB_CSS_TYPE_SELECTOR
}
lxb_css_type_t;

typedef struct {
    lxb_css_type_t type;
    void           *next;
    void           *prev;
    void           *parent;
}
lxb_css_node_t;


#endif /* LXB_CSS_NODE_H */
