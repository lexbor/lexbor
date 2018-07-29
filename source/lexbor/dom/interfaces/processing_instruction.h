/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_DOM_PROCESSING_INSTRUCTION_H
#define LEXBOR_DOM_PROCESSING_INSTRUCTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/dom/interfaces/document.h"
#include "lexbor/dom/interfaces/text.h"


struct lxb_dom_processing_instruction {
    lxb_dom_character_data_t char_data;

    lexbor_str_t             target;
};


lxb_dom_processing_instruction_t *
lxb_dom_processing_instruction_create(lxb_dom_document_t *document);

lxb_dom_processing_instruction_t *
lxb_dom_processing_instruction_destroy(lxb_dom_processing_instruction_t *processing_instruction);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_PROCESSING_INSTRUCTION_H */
