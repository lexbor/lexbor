/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_DOM_EVENT_TARGET_H
#define LEXBOR_DOM_EVENT_TARGET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/dom/interface.h"


struct lxb_dom_event_target {
    void *events;
};


lxb_dom_event_target_t *
lxb_dom_event_target_create(lxb_dom_document_t *document);

lxb_dom_event_target_t *
lxb_dom_event_target_destroy(lxb_dom_event_target_t *event_target,
                             lxb_dom_document_t *document);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_EVENT_TARGET_H */
