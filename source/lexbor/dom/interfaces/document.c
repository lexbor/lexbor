/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/interfaces/document.h"


lxb_dom_document_t *
lxb_dom_document_interface_create(lxb_dom_document_t *document)
{
    lxb_dom_document_t *element;

    element = lexbor_mraw_calloc(document->mraw,
                                 sizeof(lxb_dom_document_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = lxb_dom_interface_document(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    element->create_interface = lxb_dom_interface_create;
    element->destroy_interface = lxb_dom_interface_destroy;

    return element;
}

lxb_dom_document_t *
lxb_dom_document_interface_destroy(lxb_dom_document_t *document)
{
    return lexbor_mraw_free(
        lxb_dom_interface_node(document)->owner_document->mraw,
        document);
}

void
lxb_dom_document_attach_doctype(lxb_dom_document_t *document,
                                lxb_dom_document_type_t *doctype)
{
    document->doctype = doctype;
}

void
lxb_dom_document_attach_element(lxb_dom_document_t *document,
                                lxb_dom_element_t *element)
{
    document->element = element;
}

/*
lxb_dom_element_t *
lxb_dom_document_create_element(lxb_dom_document_t *document,
                                const lxb_char_t *local_name, size_t lname_len,
                                void *reserved_for_opt)
{
//    return lxb_dom_element_create(document);
    return NULL;
}

//lxb_dom_text_t *
//lxb_dom_document_create_text_node(lxb_dom_document_t *document,
//                                  const lxb_char_t *data, size_t data_len)
//{
//    lxb_dom_text_t *text;
//
//    text = (lxb_dom_text_t *) lxb_html_create_node(document, LXB_TAG__TEXT,
//                                                   LXB_NS_HTML);
//    if (text == NULL) {
//        return NULL;
//    }
//
//    lexbor_str_init(&text->char_data.data,
//                    lxb_html_document_mraw_text(document), data_len);
//    if (text->char_data.data.data == NULL) {
//        lxb_dom_text_destroy(text);
//
//        return NULL;
//    }
//
//    lexbor_str_append(&text->char_data.data,
//                      lxb_html_document_mraw_text(document), data, data_len);
//
//    return text;
//}
*/
