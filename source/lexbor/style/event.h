/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_EVENT_H
#define LEXBOR_STYLE_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/style/base.h"
#include "lexbor/html/html.h"
#include "lexbor/css/css.h"


LXB_API lxb_status_t
lxb_style_event_insert(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_event_insert_element(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_event_insert_attribute(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_event_remove(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_event_remove_element(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_event_remove_attribute(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_event_destroy(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_event_destroy_element(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_event_destroy_attribute(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_style_event_set_value(lxb_dom_node_t *node,
                          const lxb_char_t *value, size_t length);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_EVENT_H */
