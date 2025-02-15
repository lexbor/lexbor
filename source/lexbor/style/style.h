/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_STYLE_H
#define LEXBOR_STYLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/style/base.h"
#include "lexbor/dom/interfaces/document.h"
#include "lexbor/html/html.h"
#include "lexbor/css/css.h"
#include "lexbor/style/dom/interfaces/document.h"
#include "lexbor/style/dom/interfaces/element.h"
#include "lexbor/style/html/interfaces/document.h"
#include "lexbor/style/html/interfaces/element.h"
#include "lexbor/style/html/interfaces/style_element.h"


LXB_API uintptr_t
lxb_style_id_by_name(const lxb_dom_document_t *doc,
                     const lxb_char_t *name, size_t size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STYLE_H */
