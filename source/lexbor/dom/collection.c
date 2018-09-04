/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/collection.h"
#include "lexbor/dom/interfaces/document.h"


lxb_dom_collection_t *
lxb_dom_collection_create(lxb_dom_document_t *document)
{
    lxb_dom_collection_t *col;

    col = lexbor_mraw_calloc(document->mraw, sizeof(lxb_dom_collection_t));
    if (col == NULL) {
        return NULL;
    }

    col->document = document;

    return col;
}

lxb_status_t
lxb_dom_collection_init(lxb_dom_collection_t *col, size_t start_list_size)
{
    if (col == NULL) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    if (col->document == NULL) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    return lexbor_array_init(&col->array, start_list_size);
}

lxb_dom_collection_t *
lxb_dom_collection_destroy(lxb_dom_collection_t *col, bool self_destroy)
{
    if (col == NULL || col->document == NULL) {
        return NULL;
    }

    if (col->array.list != NULL) {
        lexbor_array_destroy(&col->array, false);

        col->array.list = NULL;
    }

    if (self_destroy) {
        return lexbor_mraw_free(col->document->mraw, col);
    }

    return col;
}
