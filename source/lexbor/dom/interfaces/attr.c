/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/interfaces/attr.h"
#include "lexbor/dom/interfaces/document.h"


lxb_dom_attr_t *
lxb_dom_attr_interface_create(lxb_dom_document_t *document)
{
    lxb_dom_attr_t *attr;

    attr = lexbor_mraw_calloc(document->mraw, sizeof(lxb_dom_attr_t));
    if (attr == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(attr);

    node->owner_document = document;
    node->type = LXB_DOM_NODE_TYPE_ATTRIBUTE;

    return attr;
}

lxb_dom_attr_t *
lxb_dom_attr_interface_destroy(lxb_dom_attr_t *attr)
{
    lxb_dom_document_t *doc = lxb_dom_interface_node(attr)->owner_document;

    if (attr->name.data != NULL) {
        lexbor_mraw_free(doc->text, attr->name.data);
    }

    if (attr->local_name.data != NULL) {
        lexbor_mraw_free(doc->text, attr->local_name.data);
    }

    if (attr->value != NULL) {
        if (attr->value->data != NULL) {
            lexbor_mraw_free(doc->text, attr->value->data);
        }

        lexbor_mraw_free(doc->mraw, attr->value);
    }

    return lexbor_mraw_free(doc->mraw, attr);
}

lxb_status_t
lxb_dom_attr_set_name(lxb_dom_attr_t *attr,
                      const lxb_char_t *local_name, size_t local_name_len,
                      const lxb_char_t *prefix, size_t prefix_len)
{
    const lxb_char_t *tmp;
    lxb_dom_document_t *doc = lxb_dom_interface_node(attr)->owner_document;

    if (attr->local_name.data == NULL) {
        lexbor_str_init(&attr->local_name, doc->text, local_name_len);
        if (attr->local_name.data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        memcpy(attr->local_name.data, local_name,
               sizeof(lxb_char_t) * local_name_len);

        attr->local_name.data[local_name_len] = 0x00;
        attr->local_name.length = local_name_len;
    }
    else {
        attr->local_name.length = 0;

        tmp = lexbor_str_append(&attr->local_name, doc->text,
                                local_name, local_name_len);
        if (tmp == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        attr->local_name.length = local_name_len;
    }

    size_t name_size = local_name_len + prefix_len + 1;

    if (attr->name.data == NULL) {
        lexbor_str_init(&attr->name, doc->text, name_size);

        if (attr->name.data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }
    else {
        attr->name.length = 0;

        if (lexbor_str_size(&attr->name) <= name_size) {
            tmp = lexbor_str_realloc(&attr->name, doc->text, (name_size + 1));
            if (tmp == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }
    }

    if (prefix_len != 0) {
        memcpy(attr->name.data, prefix, sizeof(lxb_char_t) * prefix_len);

        /* U+003A COLON (:) */
        attr->name.data[prefix_len] = 0x3A;

        memcpy(&attr->name.data[(prefix_len + 1)],
               local_name, sizeof(lxb_char_t) * local_name_len);

        attr->name.data[name_size] = 0x00;
        attr->name.length = name_size;
    }
    else {
        memcpy(attr->name.data,
               local_name, sizeof(lxb_char_t) * local_name_len);

        attr->name.data[local_name_len] = 0x00;
        attr->name.length = local_name_len;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_attr_set_value(lxb_dom_attr_t *attr,
                       const lxb_char_t *value, size_t value_len)
{
    lxb_dom_document_t *doc = lxb_dom_interface_node(attr)->owner_document;

    if (attr->value == NULL) {
        attr->value = lexbor_mraw_calloc(doc->mraw, sizeof(lexbor_str_t));
        if (attr->value == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    if (attr->value->data == NULL) {
        lexbor_str_init(attr->value, doc->text, value_len);
        if (attr->value->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }
    else {
        attr->value->length = 0;

        if (lexbor_str_size(attr->value) <= value_len) {
            const lxb_char_t *tmp;

            tmp = lexbor_str_realloc(attr->value, doc->text, (value_len + 1));
            if (tmp == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }
    }

    memcpy(attr->value->data, value, sizeof(lxb_char_t) * value_len);

    attr->value->data[value_len] = 0x00;
    attr->value->length = value_len;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_attr_clone_name_value(lxb_dom_attr_t *attr_from,
                              lxb_dom_attr_t *attr_to)
{
    lxb_dom_document_t *doc = lxb_dom_interface_node(attr_to)->owner_document;

    if (attr_to->name.data == NULL) {
        lexbor_str_init(&attr_to->name, doc->text, attr_from->name.length);
        if (attr_to->name.data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }
    else {
        attr_to->name.length = 0;

        if (lexbor_str_size(&attr_to->name) <= attr_from->name.length) {
            const lxb_char_t *tmp;

            tmp = lexbor_str_realloc(&attr_to->name,
                                     doc->text, (attr_from->name.length + 1));
            if (tmp == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }
    }

    memcpy(attr_to->name.data,
           attr_from->name.data, sizeof(lxb_char_t) * attr_from->name.length);

    attr_to->name.data[ attr_from->name.length ] = 0x00;
    attr_to->name.length = attr_from->name.length;

    if (attr_to->local_name.data == NULL) {
        lexbor_str_init(&attr_to->local_name, doc->text,
                        attr_from->local_name.length);

        if (attr_to->local_name.data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }
    else {
        attr_to->local_name.length = 0;

        if (lexbor_str_size(&attr_to->local_name) <= attr_from->local_name.length) {
            const lxb_char_t *tmp;

            tmp = lexbor_str_realloc(&attr_to->local_name, doc->text,
                                     (attr_from->local_name.length + 1));
            if (tmp == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }
    }

    memcpy(attr_to->local_name.data, attr_from->local_name.data,
           sizeof(lxb_char_t) * attr_from->local_name.length);

    attr_to->local_name.data[ attr_from->local_name.length ] = 0x00;
    attr_to->local_name.length = attr_from->local_name.length;

    if (attr_from->value != NULL) {
        return lxb_dom_attr_set_value(attr_to, attr_from->value->data,
                                      attr_from->value->length);
    }

    return LXB_STATUS_OK;
}


bool
lxb_dom_attr_compare(lxb_dom_attr_t *first, lxb_dom_attr_t *second)
{
    if (first->name.length == second->name.length
        && first->node.ns == second->node.ns
        && lexbor_str_data_ncasecmp(first->name.data, second->name.data,
                                    second->name.length))
    {
        if (first->value == second->value) {
            return true;
        }

        if (first->value == NULL || second->value == NULL) {
            return false;
        }

        if (first->value->length == second->value->length
            && lexbor_str_data_ncasecmp(first->value->data, second->value->data,
                                        second->value->length))
        {
            return true;
        }

        return false;
    }

    return false;
}
