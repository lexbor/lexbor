/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/interfaces/element.h"
#include "lexbor/dom/interfaces/attr.h"
#include "lexbor/ns/ns.h"


static lexbor_action_t
lxb_dom_elements_by_tag_name_cb(lxb_dom_node_t *node, void *ctx);

static lexbor_action_t
lxb_dom_elements_by_tag_name_cb_all(lxb_dom_node_t *node, void *ctx);


typedef struct {
    lxb_dom_collection_t *col;
    lxb_status_t         status;
    lxb_tag_id_t         tag_id;
    lxb_ns_id_t          ns_id;

    size_t               qname_len;
    size_t               prefix_len;
    size_t               lname_len;

    const lxb_char_t     *local_name;
    const lxb_char_t     *qualified_name;
}
lxb_dom_element_cb_ctx_t;


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

lxb_dom_element_t *
lxb_dom_element_create(lxb_dom_document_t *document,
                       const lxb_char_t *local_name, size_t lname_len,
                       const lxb_char_t *ns_link, size_t ns_len,
                       const lxb_char_t *prefix, size_t prefix_len,
                       const lxb_char_t *is, size_t is_len,
                       bool sync_custom, bool lowercase)
{
    lxb_status_t status;
    const lxb_ns_data_t *ns_data;
    const lxb_tag_data_t *tag_data;
    lxb_dom_interface_t *interface;

    ns_data = lxb_ns_find_or_append(document->ns, ns_link, ns_len, NULL, 0);
    if (ns_data == NULL) {
        return NULL;
    }

    tag_data = lxb_tag_find_or_append(document->tags, local_name, lname_len);
    if (tag_data == NULL) {
        return NULL;
    }

    /* TODO: Must implement custom elements */

    /* 7. Otherwise */
    interface = lxb_dom_document_create_interface(document, tag_data->tag_id,
                                                  ns_data->ns_id);
    if (interface == NULL) {
        return NULL;
    }

    if (prefix_len != 0 || lowercase == false) {
        status = lxb_dom_element_qualified_name_set(interface, prefix,
                                                    (unsigned int) prefix_len,
                                                    local_name,
                                                    (unsigned int) lname_len);
        if (status != LXB_STATUS_OK) {
            return lxb_dom_document_destroy_interface(interface);
        }
    }

    if (is_len != 0) {
        status = lxb_dom_element_is_set(interface, is, is_len);
        if (status != LXB_STATUS_OK) {
            return lxb_dom_document_destroy_interface(interface);
        }
    }

    if (ns_data->ns_id == LXB_NS_HTML && is_len != 0) {
        lxb_dom_interface_element(interface)->custom_state =
            LXB_DOM_ELEMENT_CUSTOM_STATE_UNDEFINED;
    }
    else {
        lxb_dom_interface_element(interface)->custom_state =
            LXB_DOM_ELEMENT_CUSTOM_STATE_UNCUSTOMIZED;
    }

    return interface;
}

bool
lxb_dom_element_has_attributes(lxb_dom_element_t *element)
{
    return element->first_attr != NULL;
}

lxb_dom_attr_t *
lxb_dom_element_set_attribute(lxb_dom_element_t *element,
                              const lxb_char_t *qualified_name, size_t qn_len,
                              const lxb_char_t *value, size_t value_len)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;

    attr = lxb_dom_element_attr_is_exist(element, qualified_name, qn_len);

    if (attr == NULL) {
        attr = lxb_dom_attr_interface_create(element->node.owner_document);
        if (attr == NULL) {
            return NULL;
        }

        if (element->node.ns == LXB_NS_HTML
            && element->node.owner_document->type == LXB_DOM_DOCUMENT_DTYPE_HTML)
        {
            status = lxb_dom_attr_set_name(attr, qualified_name, qn_len,
                                           NULL, 0, true);
        }
        else {
            status = lxb_dom_attr_set_name(attr, qualified_name, qn_len,
                                           NULL, 0, false);
        }

        if (status != LXB_STATUS_OK) {
            return lxb_dom_attr_interface_destroy(attr);
        }

        status = lxb_dom_attr_set_value(attr, value, value_len);
        if (status != LXB_STATUS_OK) {
            return lxb_dom_attr_interface_destroy(attr);
        }

        lxb_dom_element_attr_append(element, attr);

        return attr;
    }

    status = lxb_dom_attr_set_value(attr, value, value_len);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    return attr;
}

const lxb_char_t *
lxb_dom_element_get_attribute(lxb_dom_element_t *element,
                              const lxb_char_t *qualified_name, size_t qn_len,
                              size_t *value_len)
{
    lxb_dom_attr_t *attr;

    attr = lxb_dom_element_attr_by_name(element, qualified_name, qn_len);
    if (attr == NULL) {
        if (value_len != NULL) {
            *value_len = 0;
        }

        return NULL;
    }

    return lxb_dom_attr_value(attr, value_len);
}

lxb_status_t
lxb_dom_element_remove_attribute(lxb_dom_element_t *element,
                                 const lxb_char_t *qualified_name, size_t qn_len)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;

    attr = lxb_dom_element_attr_by_name(element, qualified_name, qn_len);
    if (attr == NULL) {
        return LXB_STATUS_OK;
    }

    status = lxb_dom_element_attr_remove(element, attr);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    lxb_dom_attr_interface_destroy(attr);

    return LXB_STATUS_OK;
}

bool
lxb_dom_element_has_attribute(lxb_dom_element_t *element,
                              const lxb_char_t *qualified_name, size_t qn_len)
{
    return lxb_dom_element_attr_by_name(element, qualified_name, qn_len) != NULL;
}

lxb_status_t
lxb_dom_element_attr_append(lxb_dom_element_t *element, lxb_dom_attr_t *attr)
{
    if (attr->name.data == NULL) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    if (element->first_attr == NULL) {
        element->first_attr = attr;
        element->last_attr = attr;

        return LXB_STATUS_OK;
    }

    attr->prev = element->last_attr;
    element->last_attr->next = attr;

    element->last_attr = attr;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_element_attr_remove(lxb_dom_element_t *element, lxb_dom_attr_t *attr)
{
    if (attr->prev != NULL) {
        attr->prev->next = attr->next;
    }
    else {
        element->first_attr = attr->next;
    }

    if (attr->next != NULL) {
        attr->next->prev = attr->prev;
    }
    else {
        element->last_attr = attr->prev;
    }

    attr->next = NULL;
    attr->prev = NULL;

    return LXB_STATUS_OK;
}

lxb_dom_attr_t *
lxb_dom_element_attr_by_name(lxb_dom_element_t *element,
                             const lxb_char_t *qualified_name, size_t qn_len)
{
    lxb_dom_attr_t *attr = element->first_attr;

    if (element->node.ns == LXB_NS_HTML
        && element->node.owner_document->type == LXB_DOM_DOCUMENT_DTYPE_HTML)
    {
        while (attr != NULL) {
            if (qn_len == attr->name.length
                && lexbor_str_data_ncasecmp(qualified_name, attr->name.data, qn_len))
            {
                return attr;
            }

            attr = attr->next;
        }

        return NULL;
    }

    while (attr != NULL) {
        if (qn_len == attr->name.length
            && lexbor_str_data_ncmp(qualified_name, attr->name.data, qn_len))
        {
            return attr;
        }

        attr = attr->next;
    }

    return NULL;
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
                              const lxb_char_t *qualified_name, size_t len)
{
    lxb_dom_attr_t *attr = element->first_attr;

    while (attr != NULL) {
        if (attr->local_name.length == len
            && lexbor_str_data_ncasecmp(attr->local_name.data,
                                        qualified_name, len))
        {
            return attr;
        }

        attr = attr->next;
    }

    return NULL;
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

bool
lxb_dom_element_qualified_name_cmp(lxb_dom_element_t *element, lxb_tag_id_t tag_id,
                                   const lxb_char_t *prefix, size_t prefix_len,
                                   const lxb_char_t *lname, size_t lname_len)
{
    if (element->node.tag_id != tag_id) {
        return NULL;
    }

    const lxb_dom_qualified_name_t *qname = element->qualified_name;

    if (prefix_len != 0) {
        if (qname == NULL || qname->prefix_len == 0) {
            return false;
        }

        if (qname->prefix_len != prefix_len
            && lexbor_str_data_ncasecmp(qname->str.data, prefix, prefix_len) == false)
        {
            return false;
        }

        if (element->node.ns == LXB_NS_HTML) {
            return true;
        }

        const lxb_char_t *data = lxb_dom_qualified_name_local_name(qname, NULL);

        if (qname->local_name_len == lname_len
            && lexbor_str_data_ncmp(data, lname, lname_len))
        {
            return true;
        }

        return false;
    }

    if (qname != NULL && qname->prefix_len != 0) {
        return false;
    }

    if (element->node.ns == LXB_NS_HTML) {
        return true;
    }

    if (qname == NULL) {
        if (lexbor_str_data_find_uppercase(lname, lname_len) == NULL) {
            return true;
        }

        return false;
    }

    if (qname->local_name_len == lname_len
        && lexbor_str_data_ncmp(qname->str.data, lname, lname_len))
    {
        return true;
    }

    return false;
}

lxb_status_t
lxb_dom_element_is_set(lxb_dom_element_t *element,
                       const lxb_char_t *is, size_t is_len)
{
    if (element->is_value == NULL) {
        element->is_value = lexbor_mraw_calloc(element->node.owner_document->mraw,
                                               sizeof(lexbor_str_t));
        if (element->is_value == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    if (element->is_value->data == NULL) {
        lexbor_str_init(element->is_value,
                        element->node.owner_document->text, is_len);

        if (element->is_value->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    if (element->is_value->length != 0) {
        element->is_value->length = 0;
    }

    lxb_char_t *data = lexbor_str_append(element->is_value,
                                         element->node.owner_document->text,
                                         is, is_len);
    if (data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_elements_by_tag_name(lxb_dom_element_t *root,
                             lxb_dom_collection_t *collection,
                             const lxb_char_t *qualified_name, size_t len)
{
    const lxb_char_t *prefix_pos;
    lxb_dom_element_cb_ctx_t cb_ctx = {0};

    cb_ctx.col = collection;

    /* "*" (U+002A) */
    if (len == 1 && *qualified_name == 0x2A) {
        lxb_dom_node_simple_walk(lxb_dom_interface_node(root),
                                 lxb_dom_elements_by_tag_name_cb_all, &cb_ctx);

        return cb_ctx.status;
    }

    cb_ctx.qname_len = len;
    cb_ctx.qualified_name = qualified_name;

    /* U+003A COLON (:) */
    prefix_pos = memchr(qualified_name, 0x3A, len);

    if (prefix_pos != NULL) {
        cb_ctx.prefix_len = prefix_pos - qualified_name;
        if (cb_ctx.prefix_len == len) {
            return LXB_STATUS_ERROR;
        }

        cb_ctx.lname_len = len - (cb_ctx.prefix_len + 1);
        cb_ctx.local_name = &qualified_name[ (cb_ctx.prefix_len + 1) ];

        cb_ctx.tag_id = lxb_tag_id_by_name(root->node.owner_document->tags,
                                           cb_ctx.local_name, cb_ctx.lname_len);
    }
    else {
        cb_ctx.tag_id = lxb_tag_id_by_name(root->node.owner_document->tags,
                                           qualified_name, len);

        cb_ctx.lname_len = len;
        cb_ctx.local_name = qualified_name;
    }

    if (cb_ctx.tag_id == LXB_TAG__UNDEF) {
        return LXB_STATUS_OK;
    }

    lxb_dom_node_simple_walk(lxb_dom_interface_node(root),
                             lxb_dom_elements_by_tag_name_cb, &cb_ctx);

    return cb_ctx.status;
}

static lexbor_action_t
lxb_dom_elements_by_tag_name_cb_all(lxb_dom_node_t *node, void *ctx)
{
    if (node->type != LXB_DOM_NODE_TYPE_ELEMENT) {
        return LEXBOR_ACTION_NEXT;
    }

    lxb_dom_element_cb_ctx_t *cb_ctx = ctx;

    cb_ctx->status = lxb_dom_collection_append(cb_ctx->col, node);
    if (cb_ctx->status != LXB_STATUS_OK) {
        return LEXBOR_ACTION_STOP;
    }

    return LEXBOR_ACTION_NEXT;
}

static lexbor_action_t
lxb_dom_elements_by_tag_name_cb(lxb_dom_node_t *node, void *ctx)
{
    if (node->type != LXB_DOM_NODE_TYPE_ELEMENT) {
        return LEXBOR_ACTION_NEXT;
    }

    bool is_it;
    lxb_dom_element_cb_ctx_t *cb_ctx = ctx;

    is_it = lxb_dom_element_qualified_name_cmp(lxb_dom_interface_element(node), cb_ctx->tag_id,
                                               cb_ctx->qualified_name, cb_ctx->prefix_len,
                                               cb_ctx->local_name, cb_ctx->lname_len);

    if (is_it) {
        cb_ctx->status = lxb_dom_collection_append(cb_ctx->col, node);

        if (cb_ctx->status != LXB_STATUS_OK) {
            return LEXBOR_ACTION_STOP;
        }
    }

    return LEXBOR_ACTION_NEXT;
}
