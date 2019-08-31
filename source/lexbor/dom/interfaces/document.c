/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/dom/interfaces/document.h"
#include "lexbor/dom/interfaces/element.h"
#include "lexbor/dom/interfaces/text.h"
#include "lexbor/dom/interfaces/document_fragment.h"
#include "lexbor/dom/interfaces/comment.h"
#include "lexbor/dom/interfaces/cdata_section.h"
#include "lexbor/dom/interfaces/cdata_section.h"
#include "lexbor/dom/interfaces/processing_instruction.h"


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

lxb_dom_element_t *
lxb_dom_document_create_element(lxb_dom_document_t *document,
                                const lxb_char_t *local_name, size_t lname_len,
                                void *reserved_for_opt)
{
    /* TODO: If localName does not match the Name production... */

    const lxb_char_t *ns_link;
    size_t ns_len;
    bool lowercase;

    if (document->type == LXB_DOM_DOCUMENT_DTYPE_HTML) {
        ns_link = (const lxb_char_t *) "http://www.w3.org/1999/xhtml";

        /* FIXME: he will get len at the compilation stage?!? */
        ns_len = strlen((const char *) ns_link);

        lowercase = true;
    }
    else {
        ns_link = NULL;
        ns_len = 0;

        lowercase = false;
    }

    return lxb_dom_element_create(document, local_name, lname_len,
                                  ns_link, ns_len, NULL, 0, NULL, 0,
                                  true, lowercase);
}

lxb_dom_element_t *
lxb_dom_document_destroy_element(lxb_dom_element_t *element)
{
    return lxb_dom_element_destroy(element);
}

lxb_dom_document_fragment_t *
lxb_dom_document_create_document_fragment(lxb_dom_document_t *document)
{
    return lxb_dom_document_fragment_interface_create(document);
}

lxb_dom_text_t *
lxb_dom_document_create_text_node(lxb_dom_document_t *document,
                                  const lxb_char_t *data, size_t len)
{
    lxb_dom_text_t *text;

    text = lxb_dom_document_create_interface(document,
                                             LXB_TAG__TEXT, LXB_NS_HTML);
    if (text == NULL) {
        return NULL;
    }

    lexbor_str_init(&text->char_data.data, document->text, len);
    if (text->char_data.data.data == NULL) {
        return lxb_dom_document_destroy_interface(text);
    }

    lexbor_str_append(&text->char_data.data, document->text, data, len);

    return text;
}

lxb_dom_cdata_section_t *
lxb_dom_document_create_cdata_section(lxb_dom_document_t *document,
                                      const lxb_char_t *data, size_t len)
{
    if (document->type != LXB_DOM_DOCUMENT_DTYPE_HTML) {
        return NULL;
    }

    const lxb_char_t *end = data + len;
    const lxb_char_t *ch = memchr(data, ']', sizeof(lxb_char_t) * len);

    while (ch != NULL) {
        if ((end - ch) < 3) {
            break;
        }

        if(memcmp(ch, "]]>", 3) == 0) {
            return NULL;
        }

        ch++;
        ch = memchr(ch, ']', sizeof(lxb_char_t) * (end - ch));
    }

    lxb_dom_cdata_section_t *cdata;

    cdata = lxb_dom_cdata_section_interface_create(document);
    if (cdata == NULL) {
        return NULL;
    }

    lexbor_str_init(&cdata->text.char_data.data, document->text, len);
    if (cdata->text.char_data.data.data == NULL) {
        return lxb_dom_cdata_section_interface_destroy(cdata);
    }

    lexbor_str_append(&cdata->text.char_data.data, document->text, data, len);

    return cdata;
}

lxb_dom_processing_instruction_t *
lxb_dom_document_create_processing_instruction(lxb_dom_document_t *document,
                                               const lxb_char_t *target, size_t target_len,
                                               const lxb_char_t *data, size_t data_len)
{
    /*
     * TODO: If target does not match the Name production,
     * then throw an "InvalidCharacterError" DOMException.
     */

    const lxb_char_t *end = data + data_len;
    const lxb_char_t *ch = memchr(data, '?', sizeof(lxb_char_t) * data_len);

    while (ch != NULL) {
        if ((end - ch) < 2) {
            break;
        }

        if(memcmp(ch, "?>", 2) == 0) {
            return NULL;
        }

        ch++;
        ch = memchr(ch, '?', sizeof(lxb_char_t) * (end - ch));
    }

    lxb_dom_processing_instruction_t *pi;

    pi = lxb_dom_processing_instruction_interface_create(document);
    if (pi == NULL) {
        return NULL;
    }

    lexbor_str_init(&pi->char_data.data, document->text, data_len);
    if (pi->char_data.data.data == NULL) {
        return lxb_dom_processing_instruction_interface_destroy(pi);
    }

    lexbor_str_init(&pi->target, document->text, target_len);
    if (pi->target.data == NULL) {
        lexbor_str_destroy(&pi->char_data.data, document->text, false);

        return lxb_dom_processing_instruction_interface_destroy(pi);
    }

    lexbor_str_append(&pi->char_data.data, document->text, data, data_len);
    lexbor_str_append(&pi->target, document->text, target, target_len);

    return pi;
}


lxb_dom_comment_t *
lxb_dom_document_create_comment(lxb_dom_document_t *document,
                                const lxb_char_t *data, size_t len)
{
    lxb_dom_comment_t *comment;

    comment = lxb_dom_document_create_interface(document, LXB_TAG__EM_COMMENT,
                                                LXB_NS_HTML);
    if (comment == NULL) {
        return NULL;
    }

    lexbor_str_init(&comment->char_data.data, document->text, len);
    if (comment->char_data.data.data == NULL) {
        return lxb_dom_document_destroy_interface(comment);
    }

    lexbor_str_append(&comment->char_data.data, document->text, data, len);

    return comment;
}
