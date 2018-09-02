/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/interfaces/element.h"
#include "lexbor/dom/interfaces/attr.h"


lxb_dom_element_t *
lxb_dom_element_interface_create(lxb_dom_document_t *document)
{
    lxb_dom_element_t *element;

    element = lexbor_mraw_calloc(document->mraw,
                                 sizeof(lxb_dom_element_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = document;
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    return element;
}

lxb_dom_element_t *
lxb_dom_element_interface_destroy(lxb_dom_element_t *element)
{
    lxb_dom_attr_t *attr_next;
    lxb_dom_attr_t *attr = element->first_attr;

    while (attr != NULL) {
        attr_next = attr->next;

        lxb_dom_attr_interface_destroy(attr);

        attr = attr_next;
    }

    return lexbor_mraw_free(
        lxb_dom_interface_node(element)->owner_document->mraw,
        element);
}

bool
lxb_dom_element_compare(lxb_dom_element_t *first, lxb_dom_element_t *second)
{
    lxb_dom_attr_t *f_attr = first->first_attr;
    lxb_dom_attr_t *s_attr = second->first_attr;

    /* Compare attr counts */
    while (f_attr != NULL && s_attr != NULL) {
        f_attr = f_attr->next;
        s_attr = s_attr->next;
    }

    if (f_attr != NULL || s_attr != NULL) {
        return false;
    }

    /* Compare attr */
    f_attr = first->first_attr;

    while (f_attr != NULL) {
        s_attr = lxb_dom_element_attr_is_exist(second, f_attr->name.data,
                                               f_attr->name.length);

        if (s_attr == NULL || lxb_dom_attr_compare(f_attr, s_attr) == false) {
            return false;
        }

        f_attr = f_attr->next;
    }

    return true;
}

lxb_dom_attr_t *
lxb_dom_element_attr_is_exist(lxb_dom_element_t *element,
                              const lxb_char_t *name, size_t len)
{
    lxb_dom_attr_t *attr = element->first_attr;

    while (attr != NULL) {
        if (attr->local_name.length == len
            && lexbor_str_data_ncasecmp(attr->local_name.data, name, len))
        {
            return attr;
        }

        attr = attr->next;
    }

    return NULL;
}

void
lxb_dom_element_attr_append(lxb_dom_element_t *element, lxb_dom_attr_t *attr)
{
    if (attr->name.data == NULL) {
        return;
    }

    if (element->first_attr == NULL) {
        element->first_attr = attr;
        element->last_attr = attr;

        return;
    }

    attr->prev = element->last_attr;
    element->last_attr->next = attr;

    element->last_attr = attr;
}

lxb_status_t
lxb_dom_element_qualified_name_set(lxb_dom_element_t *element,
                                   const lxb_char_t *prefix, unsigned int prefix_len,
                                   const lxb_char_t *lname, unsigned int lname_len)
{
    if (lname_len == 0) {
        lname = lxb_tag_name_by_id(lxb_dom_interface_node(element)->owner_document->tags,
                                   lxb_dom_interface_node(element)->tag_id,
                                   (size_t *) &lname_len);
    }

    if (element->qualified_name != NULL) {
        return lxb_dom_qualified_name_change(element->node.owner_document,
                                             element->qualified_name,
                                             prefix, prefix_len,
                                             lname, lname_len);
    }

    element->qualified_name = lxb_dom_qualified_name_make(element->node.owner_document,
                                                          prefix, prefix_len,
                                                          lname, lname_len);
    if (element->qualified_name == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}
