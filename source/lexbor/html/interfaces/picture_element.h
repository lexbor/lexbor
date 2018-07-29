/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_HTML_PICTURE_ELEMENT_H
#define LEXBOR_HTML_PICTURE_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/interface.h"
#include "lexbor/html/interfaces/element.h"


struct lxb_html_picture_element {
    lxb_html_element_t element;
};


lxb_html_picture_element_t *
lxb_html_picture_element_create(lxb_html_document_t *document);

lxb_html_picture_element_t *
lxb_html_picture_element_destroy(lxb_html_picture_element_t *picture_element);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_PICTURE_ELEMENT_H */
