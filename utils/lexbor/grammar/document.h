/*
 * Copyright (C) 2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_GRAMMAR_DOCUMENT_H
#define LEXBOR_GRAMMAR_DOCUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/interfaces/document.h"


typedef lxb_html_document_t lxb_grammar_document_t;


lxb_inline lxb_grammar_document_t *
lxb_grammar_document_destroy(lxb_grammar_document_t *document)
{
    (void) lxb_html_document_destroy(document);

    return NULL;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_GRAMMAR_DOCUMENT_H */

