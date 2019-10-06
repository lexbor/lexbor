/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/dom/interfaces/character_data.h"
#include "lexbor/dom/interfaces/document.h"


lxb_dom_character_data_t *
lxb_dom_character_data_interface_create(lxb_dom_document_t *document)
{
    lxb_dom_character_data_t *element;

    element = lexbor_mraw_calloc(document->mraw,
                                 sizeof(lxb_dom_character_data_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = document;
    node->type = LXB_DOM_NODE_TYPE_UNDEF;

    return element;
}

lxb_dom_character_data_t *
lxb_dom_character_data_interface_destroy(lxb_dom_character_data_t *character_data)
{
    lexbor_str_destroy(&character_data->data,
                       lxb_dom_interface_node(character_data)->owner_document->text,
                       false);

    return lexbor_mraw_free(
        lxb_dom_interface_node(character_data)->owner_document->mraw,
        character_data);
}

/* TODO: oh, need to... https://dom.spec.whatwg.org/#concept-cd-replace */
lxb_status_t
lxb_dom_character_data_replace(lxb_dom_character_data_t *ch_data,
                               const lxb_char_t *data, size_t len,
                               size_t offset, size_t count)
{
    if (ch_data->data.data == NULL) {
        lexbor_str_init(&ch_data->data, ch_data->node.owner_document->text, len);
        if (ch_data->data.data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }
    else if (lexbor_str_size(&ch_data->data) < len) {
        const lxb_char_t *data;

        data = lexbor_str_realloc(&ch_data->data,
                                  ch_data->node.owner_document->text, (len + 1));
        if (data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    memcpy(ch_data->data.data, data, sizeof(lxb_char_t) * len);

    ch_data->data.data[len] = 0x00;
    ch_data->data.length = len;

    return LXB_STATUS_OK;
}
