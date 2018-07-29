/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/interfaces/element.h"


lxb_dom_element_t *
lxb_dom_element_create(lxb_dom_document_t *document)
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
lxb_dom_element_destroy(lxb_dom_element_t *element)
{
    return lexbor_mraw_free(
        lxb_dom_interface_node(element)->owner_document->mraw,
        element);
}

bool
lxb_dom_element_compare(lxb_dom_element_t *first, lxb_dom_element_t *second)
{
    lxb_dom_element_attr_t *f_attr = first->first_attr;
    lxb_dom_element_attr_t *s_attr = second->first_attr;

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

        if (s_attr == NULL
            || lxb_dom_element_attr_compare(f_attr, s_attr) == false)
        {
            return false;
        }

        f_attr = f_attr->next;
    }

    return true;
}

bool
lxb_dom_element_attr_compare(lxb_dom_element_attr_t *first,
                             lxb_dom_element_attr_t *second)
{
    if (first->name.length == second->name.length
        && first->value.length == second->value.length
        && first->ns == second->ns
        && lexbor_str_data_ncasecmp(first->name.data, second->name.data,
                                    second->name.length))
    {
        if (first->value.data == second->value.data) {
            return true;
        }

        if (first->value.data != NULL && second->value.data != NULL
            && lexbor_str_data_ncasecmp(first->value.data, second->value.data,
                                        second->value.length))
        {
            return true;
        }

        return false;
    }

    return false;
}

lxb_dom_element_attr_t *
lxb_dom_element_attr_is_exist(lxb_dom_element_t *element,
                              const lxb_char_t *name, size_t len)
{
    lxb_dom_element_attr_t *attr = element->first_attr;

    while (attr != NULL) {
        if (attr->name.length == len
            && lexbor_str_data_ncasecmp(attr->name.data, name, len))
        {
            return attr;
        }

        attr = attr->next;
    }

    return NULL;
}

lxb_dom_element_attr_t *
lxb_dom_element_attr_create(lxb_dom_element_t *element)
{
    lxb_dom_element_attr_t *attr;

    attr = lexbor_mraw_calloc(element->node.owner_document->mraw,
                              sizeof(lxb_dom_element_attr_t));
    if (attr == NULL) {
        return NULL;
    }

    attr->owner = element;

    return attr;
}

lxb_dom_element_t *
lxb_dom_element_attr_destroy(lxb_dom_element_attr_t *attr)
{
    return lexbor_mraw_free(attr->owner->node.owner_document->mraw, attr);
}

void
lxb_dom_element_attr_append(lxb_dom_element_t *element,
                            lxb_dom_element_attr_t *attr)
{
    if (attr->name.data == NULL || attr->local_name.data == NULL) {
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
