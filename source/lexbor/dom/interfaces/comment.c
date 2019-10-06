/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/dom/interfaces/comment.h"
#include "lexbor/dom/interfaces/document.h"


lxb_dom_comment_t *
lxb_dom_comment_interface_create(lxb_dom_document_t *document)
{
    lxb_dom_comment_t *element;

    element = lexbor_mraw_calloc(document->mraw,
                                 sizeof(lxb_dom_comment_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = document;
    node->type = LXB_DOM_NODE_TYPE_COMMENT;

    return element;
}

lxb_dom_comment_t *
lxb_dom_comment_interface_destroy(lxb_dom_comment_t *comment)
{
    return lexbor_mraw_free(
        lxb_dom_interface_node(comment)->owner_document->mraw,
        comment);
}
