/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/html.h"
#include "lexbor/html/tree.h"
#include "lexbor/html/interfaces/unknown_element.h"

bool
lxb_html_node_is_void_noi(lxb_dom_node_t *node)
{
    return lxb_html_node_is_void(node);
}
