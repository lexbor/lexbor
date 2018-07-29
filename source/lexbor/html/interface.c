/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/core/mraw.h"

#include "lexbor/html/interface.h"
#include "lexbor/html/interfaces/document.h"


void *
lxb_html_interface_destroy(lxb_html_document_t *document, void *interface)
{
    if (document == NULL || document->mem == NULL) {
        return NULL;
    }

    return lexbor_mraw_free(document->mem->mraw, interface);
}
