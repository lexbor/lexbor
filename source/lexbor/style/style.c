/*
 * Copyright (C) 2025-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/style.h"
#include "lexbor/html/interfaces/document.h"


uintptr_t
lxb_style_id_by_name(const lxb_dom_document_t *doc,
                     const lxb_char_t *name, size_t size)
{
    const lxb_css_entry_data_t *data;

    data = lxb_css_property_by_name(name, size);

    if (data == NULL) {
        return lxb_dom_document_css_customs_find_id(doc, name, size);
    }

    return data->unique;
}

lxb_status_t
lxb_style_init(lxb_html_document_t *doc)
{
    if (doc == NULL) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    doc->done = lxb_html_document_done_cb;

    lxb_html_document_style_mutation_init(doc);

    return lxb_dom_document_css_init(lxb_dom_interface_document(doc), false);
}

void
lxb_style_destroy(lxb_html_document_t *doc)
{
    doc->done = lxb_html_document_done_cb;

    lxb_html_document_style_mutation_erase(doc);

    lxb_dom_document_css_destroy(lxb_dom_interface_document(doc));
}
