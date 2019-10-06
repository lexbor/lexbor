/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/%%prefix%%/interfaces/%%name%%.h"
#include "lexbor/%%prefix%%/interfaces/document.h"


%%type%% *
lxb_%%prefix%%_%%name%%_create(lxb_%%prefix%%_document_t *document)
{
    %%type%% *element;

    element = lexbor_mraw_calloc(%%mraw%%,
                                 sizeof(%%type%%));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = lxb_dom_interface_document(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    return element;
}

%%type%% *
lxb_%%prefix%%_%%name%%_destroy(%%type%% *%%name%%)
{
    return lexbor_mraw_free(
        lxb_dom_interface_node(%%name%%)->owner_document->mraw,
        %%name%%);
}
