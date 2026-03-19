/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_ELEMENT_STEPS_H
#define LEXBOR_STYLE_ELEMENT_STEPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/dom/interfaces/node.h"
#include "lexbor/dom/interfaces/document.h"
#include "lexbor/tag/tag.h"


LXB_API lxb_status_t
lxb_style_element_steps_insertion(lxb_dom_node_t *inserted_node);

LXB_API lxb_status_t
lxb_style_element_steps_removing(lxb_dom_node_t *removed_node,
                                 lxb_dom_node_t *old_parent);

LXB_API lxb_status_t
lxb_style_element_steps_moving(lxb_dom_node_t *moved_node,
                               lxb_dom_node_t *old_parent);

LXB_API lxb_status_t
lxb_style_element_steps_destroy(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_element_steps_children_changed(lxb_dom_node_t *parent);

LXB_API lxb_status_t
lxb_style_element_steps_post_connection(lxb_dom_node_t *connected_node);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_ELEMENT_STEPS_H */
