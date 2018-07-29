/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/interfaces/processing_instruction.h"
#include "lexbor/dom/interfaces/document.h"


lxb_dom_processing_instruction_t *
lxb_dom_processing_instruction_create(lxb_dom_document_t *document)
{
    lxb_dom_processing_instruction_t *element;

    element = lexbor_mraw_calloc(document->mraw,
                                 sizeof(lxb_dom_processing_instruction_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = document;
    node->type = LXB_DOM_NODE_TYPE_PROCESSING_INSTRUCTION;

    return element;
}

lxb_dom_processing_instruction_t *
lxb_dom_processing_instruction_destroy(lxb_dom_processing_instruction_t *processing_instruction)
{
    return lexbor_mraw_free(
        lxb_dom_interface_node(processing_instruction)->owner_document->mraw,
        processing_instruction);
}
